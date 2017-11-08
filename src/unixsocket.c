/**
 *      Copyright 2008 Fred Chien <cfsghost@gmail.com>
 *      Copyright (c) 2010 LxDE Developers, see the file AUTHORS for details.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "lxterminal.h"
#include "unixsocket.h"

typedef struct _client_info {
    LXTermWindow * lxtermwin;
    int fd;
} ClientInfo;

static gboolean init(LXTermWindow* lxtermwin, gint argc, gchar** argv);
static void start_controller(struct sockaddr_un* sock_addr, LXTermWindow* lxtermwin, int fd);
static void send_msg_to_controller(int fd, gint argc, gchar** argv);
static gboolean handle_client(GIOChannel* source, GIOCondition condition, LXTermWindow* lxtermwin);
static void accept_client(GIOChannel* source, LXTermWindow* lxtermwin);
static gboolean handle_request(GIOChannel* gio, GIOCondition condition, ClientInfo* info);

static gboolean init(LXTermWindow* lxtermwin, gint argc, gchar** argv) {
    /* Normally, LXTerminal uses one process to control all of its windows.
     * The first process to start will create a Unix domain socket in
     * g_get_user_runtime_dir().  It will then bind and listen on this socket.
     * The subsequent processes will connect to the controller that owns the
     * Unix domain socket.  They will pass their command line over the socket
     * and exit.
     *
     * If for any reason both the connect and bind fail, we will fall back to
     * having that process be standalone; it will not be either the controller
     * or a user of the controller.
     *
     * This function returns TRUE if this process should keep running and FALSE
     * if it should exit. */

    /* Formulate the path for the Unix domain socket. */
#if GLIB_CHECK_VERSION (2, 28, 0)
    gchar * socket_path = g_strdup_printf("%s/.lxterminal-socket-%s",
            g_get_user_runtime_dir(),
            gdk_display_get_name(gdk_display_get_default()));
#else
    gchar * socket_path = g_strdup_printf("%s/.lxterminal-socket-%s",
            g_get_user_cache_dir(),
            gdk_display_get_name(gdk_display_get_default()));
#endif

    /* Create socket. */
    int fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        g_warning("Socket create failed: %s\n", g_strerror(errno));
        goto err_socket;
    }

    /* Initialize socket address for Unix domain socket. */
    struct sockaddr_un sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    snprintf(sock_addr.sun_path, sizeof(sock_addr.sun_path), "%s", socket_path);

    /* Try to connect to an existing LXTerminal process. */
    if (connect(fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) == 0) {
        g_free(socket_path);
        send_msg_to_controller(fd, argc, argv);
        return FALSE;
    }

    unlink(socket_path);
    g_free(socket_path);
    start_controller(&sock_addr, lxtermwin, fd);
    return TRUE;

err_socket:
    g_free(socket_path);
    return TRUE;
}

static void start_controller(struct sockaddr_un* sock_addr, LXTermWindow* lxtermwin, int fd) {
    /* Bind to socket. */
    if (bind(fd, (struct sockaddr*) sock_addr, sizeof(*sock_addr)) < 0) {
        g_warning("Bind on socket failed: %s\n", g_strerror(errno));
        goto err_close_fd;
    }

    /* Listen on socket. */
    if (listen(fd, 5) < 0) {
        g_warning("Listen on socket failed: %s\n", g_strerror(errno));
        goto err_close_fd;
    }

    /* Create a glib I/O channel. */
    GIOChannel * gio = g_io_channel_unix_new(fd);
    if (gio == NULL) {
        g_warning("Cannot create GIOChannel\n");
        goto err_close_fd;
    }

    /* Set up GIOChannel. */
    g_io_channel_set_encoding(gio, NULL, NULL);
    g_io_channel_set_buffered(gio, FALSE);
    g_io_channel_set_close_on_unref(gio, TRUE);

    /* Add I/O channel to the main event loop. */
    if (!g_io_add_watch(gio, G_IO_IN | G_IO_HUP, (GIOFunc) handle_client, lxtermwin)) {
        g_warning("Cannot add watch on GIOChannel\n");
        goto err_gio_watch;
    }

    /* Channel will automatically shut down when the watch returns FALSE. */
    g_io_channel_unref(gio);
    return;

err_gio_watch:
    g_io_channel_unref(gio);

err_close_fd:
    close(fd);
    return;
}

static void send_msg_to_controller(int fd, gint argc, gchar** argv) {
    /* Create a glib I/O channel. */
    GIOChannel * gio = g_io_channel_unix_new(fd);
    g_io_channel_set_encoding(gio, NULL, NULL);

    /* Push current dir in case it is needed later */
    gchar * cur_dir = g_get_current_dir();
    g_io_channel_write_chars(gio, cur_dir, -1, NULL, NULL);

    /* Use "" as a pointer to '\0' since g_io_channel_write_chars() won't
     * accept NULL */
    g_io_channel_write_chars(gio, "", 1, NULL, NULL);
    g_free(cur_dir);

    /* push all of argv. */
    gint i;
    for (i = 0; i < argc; i ++) {
        g_io_channel_write_chars(gio, argv[i], -1, NULL, NULL);
        g_io_channel_write_chars(gio, "", 1, NULL, NULL);
    }

    g_io_channel_flush(gio, NULL);
    g_io_channel_unref(gio);
    close(fd);
}

static gboolean handle_client(GIOChannel* source, GIOCondition condition, LXTermWindow* lxtermwin) {
    if (condition & G_IO_IN) {
        accept_client(source, lxtermwin);
    }

    if (condition & G_IO_HUP) {
        g_error("Server listening socket closed unexpectedly\n");
    }

    return TRUE;
}

static void accept_client(GIOChannel* source, LXTermWindow* lxtermwin) {
    /* Accept the new connection. */
    int fd = accept(g_io_channel_unix_get_fd(source), NULL, NULL);
    if (fd < 0) {
        g_warning("Accept failed: %s\n", g_strerror(errno));
        return;
    }

    /* Add O_NONBLOCK to the flags. */
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

    /* Create a glib I/O channel. */
    GIOChannel * gio = g_io_channel_unix_new(fd);
    if (gio == NULL) {
        g_warning("Cannot create new GIOChannel\n");
        return;
    }

    ClientInfo* info = g_malloc(sizeof(ClientInfo));
    info->lxtermwin = lxtermwin;
    info->fd = fd;

    /* Set up the glib I/O channel and add it to the event loop. */
    g_io_channel_set_encoding(gio, NULL, NULL);
    g_io_add_watch(gio, G_IO_IN | G_IO_HUP, (GIOFunc) handle_request, info);
    g_io_channel_unref(gio);
}

static gboolean handle_request(GIOChannel* gio, GIOCondition condition, ClientInfo* info) {
    LXTermWindow * lxtermwin = info->lxtermwin;
    int fd = info->fd;
    /* Read message. */
    gchar * msg = NULL;
    gsize len = 0;
    GError * err = NULL;
    GIOStatus ret = g_io_channel_read_to_end(gio, &msg, &len, &err);
    if (ret == G_IO_STATUS_ERROR) {
        g_warning("Error reading socket: %s\n", err->message);
    }

    /* Process message. */
    if (len > 0) {
        /* Skip the the first (cur_dir) and last '\0' for argument count */
        gint argc = -1;
        gsize i;
        for (i = 0; i < len; i ++) {
            if (msg[i] == '\0') {
                argc ++;
            }
        }
        gchar * cur_dir = msg;
        gchar * * argv = g_malloc(argc * sizeof(char *));
        gint nul_count = 0;
        for (i = 0; i < len; i ++) {
            if (msg[i] == '\0' && nul_count < argc) {
                argv[nul_count] = &msg[i + 1];
                nul_count ++;
            }
        }

        /* Parse arguments.
         * Initialize a new LXTerminal and create a new window. */
        CommandArguments arguments;
        lxterminal_process_arguments(argc, argv, &arguments);
        g_free(argv);

        /* Make sure working directory matches that of the client process */
        if (arguments.working_directory == NULL) {
            arguments.working_directory = g_strdup(cur_dir);
        }
        lxterminal_initialize(lxtermwin, &arguments);
    }

    if (condition & G_IO_HUP) {
        g_free(msg);
        g_free(info);
        close(fd);
        return FALSE;
    }

    return TRUE;
}

gboolean lxterminal_socket_initialize(LXTermWindow* lxtermwin, gint argc, gchar** argv) {
    return init(lxtermwin, argc, argv);
}
