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
    GError * err = NULL;
    GIOStatus ret = g_io_channel_read_to_end(gio, &msg, &len, &err);
    if (ret == G_IO_STATUS_ERROR)
    {
        g_warning("Error reading socket: %s\n", err->message);
    }

    /* Process message. */
    if (len > 0)
    {
	/* Skip the the first (cur_dir) and last '\0' for argument count */
        gint argc = -1;
	gsize i;
	for (i = 0; i < len; i ++)
	{
	    if (msg[i] == '\0')
	    {
	    	argc ++;
	    }
	}
	gchar * cur_dir = msg;
	gchar * * argv = g_malloc(argc * sizeof(char *));
	gint nul_count = 0;
	for (i = 0; i < len; i ++)
	{
	    if (msg[i] == '\0' && nul_count < argc)
	    {
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
	if (arguments.working_directory == NULL)
	{
	    arguments.working_directory = g_strdup(cur_dir);
	}
        lxterminal_initialize(lxtermwin, &arguments);
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

gboolean lxterminal_socket_initialize(LXTermWindow * lxtermwin, gint argc, gchar * * argv)
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

        /* Push current dir in case it is needed later */
	gchar * cur_dir = g_get_current_dir();
        g_io_channel_write_chars(gio, cur_dir, -1, NULL, NULL);
	/* Use "" as a pointer to '\0' since g_io_channel_write_chars() won't accept NULL */
	g_io_channel_write_chars(gio, "", 1, NULL, NULL);
	g_free(cur_dir);

        /* push all of argv. */
	gint i;
	for (i = 0; i < argc; i ++)
	{
            g_io_channel_write_chars(gio, argv[i], -1, NULL, NULL);
	    g_io_channel_write_chars(gio, "", 1, NULL, NULL);
	}

        g_io_channel_flush(gio, NULL);
        g_io_channel_unref(gio);
        return FALSE;
    }
}
