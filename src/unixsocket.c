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
#include <glib.h>
#include <gtk/gtk.h>

#include "lxterminal.h"
#include "unixsocket.h"

static gboolean lxterminal_socket_read_channel(GIOChannel * gio, GIOCondition condition, LXTermWindow * lxtermwin);
static gboolean lxterminal_socket_accept_client(GIOChannel * source, GIOCondition condition, LXTermWindow * lxtermwin);

/* Handler for successful read on communication socket. */
static gboolean lxterminal_socket_read_channel(GIOChannel * gio, GIOCondition condition, LXTermWindow * lxtermwin)
{
    /* Read message. */
    gchar * msg = NULL;
    gsize len = 0;
    gsize term = 0;
    GError * err = NULL;
    GIOStatus ret = g_io_channel_read_line(gio, &msg, &len, &term, &err);
    if (ret == G_IO_STATUS_ERROR)
    {
        g_warning("Error reading socket: %s\n", err->message);
    }

    /* Process message. */
    if (len > 0)
    {
        /* Overwrite the line termination with a NUL. */
        msg[term] = '\0';

        /* Parse arguments.
         * Initialize a new LXTerminal and create a new window. */
        gint argc;
        gchar * * argv;
        g_shell_parse_argv(msg, &argc, &argv, NULL);
        CommandArguments arguments;
        lxterminal_process_arguments(argc, argv, &arguments);
        lxterminal_initialize(lxtermwin, &arguments);
        g_strfreev(argv);
    }
    g_free(msg);

    /* If there was a disconnect, discontinue read.  Otherwise, continue. */
    if (condition & G_IO_HUP)
    {
        return FALSE;
    }
    return TRUE;
}

/* Handler for successful listen on communication socket. */
static gboolean lxterminal_socket_accept_client(GIOChannel * source, GIOCondition condition, LXTermWindow * lxtermwin)
{
    if (condition & G_IO_IN)
    {
        /* Accept the new connection. */
        int fd = accept(g_io_channel_unix_get_fd(source), NULL, NULL);
        if (fd < 0)
            g_warning("Accept failed: %s\n", g_strerror(errno));

        /* Add O_NONBLOCK to the flags. */
        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

        /* Create a glib I/O channel. */
        GIOChannel * gio = g_io_channel_unix_new(fd);
        if (gio == NULL)
            g_warning("Cannot create new GIOChannel\n");
        else
        {
            /* Set up the glib I/O channel and add it to the event loop. */
            g_io_channel_set_encoding(gio, NULL, NULL);
            g_io_add_watch(gio, G_IO_IN | G_IO_HUP, (GIOFunc) lxterminal_socket_read_channel, lxtermwin);
            g_io_channel_unref(gio);
        }
    }

    /* Our listening socket hung up - we are dead. */
    if (condition & G_IO_HUP)
        g_error("Server listening socket closed unexpectedly\n");

    return TRUE;
}

gboolean lxterminal_socket_initialize(LXTermWindow * lxtermwin, CommandArguments * arguments)
{
    /* Normally, LXTerminal uses one process to control all of its windows.
     * The first process to start will create a Unix domain socket in /tmp.
     * It will then bind and listen on this socket.
     * The subsequent processes will connect to the controller that owns the Unix domain socket.
     * They will pass their command line over the socket and exit.
     *
     * If for any reason both the connect and bind fail, we will fall back to having that
     * process be standalone; it will not be either the controller or a user of the controller.
     * This behavior was introduced in response to a problem report (2973537).
     *
     * This function returns TRUE if this process should keep running and FALSE if it should exit. */

    /* Formulate the path for the Unix domain socket. */
    gchar * socket_path = g_strdup_printf("/tmp/.lxterminal-socket%s-%s", gdk_get_display(), g_get_user_name());

    /* Create socket. */
    int fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
    {
        g_warning("Socket create failed: %s\n", g_strerror(errno));
        g_free(socket_path);
        return TRUE;
    }

    /* Initialize socket address for Unix domain socket. */
    struct sockaddr_un sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    snprintf(sock_addr.sun_path, sizeof(sock_addr.sun_path), "%s", socket_path);

    /* Try to connect to an existing LXTerminal process. */
    if (connect(fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0)
    {
        /* Connect failed.  We are the controller, unless something fails. */
        unlink(socket_path);
        g_free(socket_path);

        /* Bind to socket. */
        if (bind(fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0)
        {
            g_warning("Bind on socket failed: %s\n", g_strerror(errno));
            close(fd);
            return TRUE;
        }

        /* Listen on socket. */
        if (listen(fd, 5) < 0)
        {
            g_warning("Listen on socket failed: %s\n", g_strerror(errno));
            close(fd);
            return TRUE;
        }

        /* Create a glib I/O channel. */
        GIOChannel * gio = g_io_channel_unix_new(fd);
        if (gio == NULL)
        {
            g_warning("Cannot create GIOChannel\n");
            close(fd);
            return TRUE;
        }

        /* Set up GIOChannel. */
        g_io_channel_set_encoding(gio, NULL, NULL);
        g_io_channel_set_buffered(gio, FALSE);
        g_io_channel_set_close_on_unref(gio, TRUE);

        /* Add I/O channel to the main event loop. */
        if ( ! g_io_add_watch(gio, G_IO_IN | G_IO_HUP, (GIOFunc) lxterminal_socket_accept_client, lxtermwin))
        {
            g_warning("Cannot add watch on GIOChannel\n");
            close(fd);
            g_io_channel_unref(gio);
            return TRUE;
        }

        /* Channel will automatically shut down when the watch returns FALSE. */
        g_io_channel_set_close_on_unref(gio, TRUE);
        g_io_channel_unref(gio);
        return TRUE;
    }
    else
    {
        g_free(socket_path);

        /* Create a glib I/O channel. */
        GIOChannel * gio = g_io_channel_unix_new(fd);
        g_io_channel_set_encoding(gio, NULL, NULL);

        /* Reissue arguments to the socket.  Start with the name of the executable. */
        g_io_channel_write_chars(gio, arguments->executable, -1, NULL, NULL);

        /* --command or -e. */
        if (arguments->command != NULL)
        {
            gchar * command = g_shell_quote(arguments->command);
            gchar * command_argument = g_strdup_printf(" --command=%s", command);
            g_io_channel_write_chars(gio, command_argument, -1, NULL, NULL);
            g_free(command);
            g_free(command_argument);
        }

        /* --geometry. */
        if ((arguments->geometry_columns != 0) && (arguments->geometry_rows != 0))
        {
            gchar * geometry = g_strdup_printf(" --geometry=%dx%d", arguments->geometry_columns, arguments->geometry_rows);
            g_io_channel_write_chars(gio, geometry, -1, NULL, NULL);
            g_free(geometry);
        }

        /* -t, -T, --title or --tabs. */
        if (arguments->tabs != NULL)
        {
            gchar * tabs = g_shell_quote(arguments->tabs);
            gchar * tabs_argument = g_strdup_printf(" --tabs=%s", tabs);
            g_io_channel_write_chars(gio, tabs_argument, -1, NULL, NULL);
            g_free(tabs);
            g_free(tabs_argument);
        }

        /* Always issue a --working-directory, either from the user's specification or the current directory. */
        gchar * working_directory = g_shell_quote((arguments->working_directory != NULL) ? arguments->working_directory : g_get_current_dir());
        gchar * working_directory_argument = g_strdup_printf(" --working-directory=%s", working_directory);
        g_io_channel_write_chars(gio, working_directory_argument, -1, NULL, NULL);
        g_free(working_directory);
        g_free(working_directory_argument);

        /* Finish up the transaction on the Unix domain socket. */
        g_io_channel_write_chars(gio, "\n", -1, NULL, NULL);
        g_io_channel_flush(gio, NULL);
        g_io_channel_unref(gio);
        return FALSE;
    }
}
