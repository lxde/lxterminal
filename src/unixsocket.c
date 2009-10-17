/*
 *      Copyright 2008 Fred Chien <cfsghost@gmail.com>
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

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "lxterminal.h"
#include "unixsocket.h"

static gboolean
lxterminal_socket_read_channel(GIOChannel *gio, GIOCondition condition, gpointer lxtermwin)
{
	GIOStatus ret;
	GError *err = NULL;
	gchar *msg;
	gsize len;
	gsize term;

	/* read messages */
	ret = g_io_channel_read_line(gio, &msg, &len, &term, &err);
	if (ret == G_IO_STATUS_ERROR)
		g_error("Error reading: %s\n", err->message);

	if (len > 0) {
		gchar **argv;
		gint argc;

		msg[term] = '\0';

		/* generate args */
		g_shell_parse_argv(msg, &argc, &argv, NULL);

		/* initializing LXTerminal and create a new window */
		lxterminal_init(lxtermwin, argc, argv, ((LXTermWindow *) lxtermwin)->setting);

		/* release */
		g_strfreev(argv);
	}
	g_free(msg);

	if (condition & G_IO_HUP)
		return FALSE;

	return TRUE;
}

static gboolean
lxterminal_socket_accept_client(GIOChannel *source, GIOCondition condition, gpointer lxtermwin)
{
	if (condition & G_IO_IN) {
		GIOChannel *gio;
		int fd;
		int flags;

		/* new connection */
		fd = accept(g_io_channel_unix_get_fd(source), NULL, NULL);
		if (fd < 0)
			g_error("Accept failed: %s\n", g_strerror(errno));

		flags = fcntl(fd, F_GETFL, 0);
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);

		gio = g_io_channel_unix_new(fd);
		if (!gio)
			g_error("Cannot create new GIOChannel!\n");

		g_io_channel_set_encoding(gio, NULL, NULL);

		g_io_add_watch(gio, G_IO_IN | G_IO_HUP, lxterminal_socket_read_channel, lxtermwin);

		g_io_channel_unref(gio);
	}

	/* our listener socket hung up - we are dead */
	if (condition & G_IO_HUP)
		g_error("Server listening socket died!\n");

	return TRUE;
}

gboolean
lxterminal_socket_init(LXTermWindow *lxtermwin, int argc, char **argv)
{
	struct sockaddr_un skaddr;
	GIOChannel *gio;
	int skfd;
	gchar *socket_path;

	socket_path = g_strdup_printf("/tmp/.lxterminal-socket%s-%s", gdk_get_display(), g_get_user_name());

	/* create socket */
	skfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (skfd < 0) {
		if (g_file_test(socket_path, G_FILE_TEST_EXISTS)) {
			unlink(socket_path);
		}

		skfd = socket(PF_UNIX, SOCK_STREAM, 0);
		if (skfd < 0)
			g_error("Cannot create socket!");
	}

	/* Initiate socket */
	bzero(&skaddr, sizeof(skaddr));

	/* setting UNIX socket */
	skaddr.sun_family = AF_UNIX;
	snprintf(skaddr.sun_path, sizeof(skaddr.sun_path), "%s", socket_path);

	/* try to connect to current LXTerminal */
	if (connect(skfd, (struct sockaddr *)&skaddr, sizeof(skaddr)) < 0) {
		unlink(socket_path);

		/* bind to socket */
		if (bind(skfd, (struct sockaddr *)&skaddr, sizeof(skaddr)) < 0)
			g_error("Bind on socket failed: %s\n", g_strerror(errno));

		/* listen on socket */
		if (listen(skfd, 5) < 0)
			g_error("Listen on socket failed: %s\n", g_strerror(errno));

		/* create I/O channel */
		gio = g_io_channel_unix_new(skfd);
		if (!gio)
			g_error("Cannot create new GIOChannel!\n");

		/* setting encoding */
		g_io_channel_set_encoding(gio, NULL, NULL);
		g_io_channel_set_buffered(gio, FALSE);
		g_io_channel_set_close_on_unref(gio, TRUE);

		/* I/O channel into the main event loop */
		if (!g_io_add_watch(gio, G_IO_IN | G_IO_HUP, lxterminal_socket_accept_client, lxtermwin))
			g_error("Cannot add watch on GIOChannel\n");

		/* channel will automatically shutdown when the watch returns FALSE */
		g_io_channel_set_close_on_unref(gio, TRUE);
		g_io_channel_unref(gio);

		g_free(socket_path);
		return TRUE;
	} else {
		int i;
		gboolean setworkdir = FALSE;

		gio = g_io_channel_unix_new(skfd);
		g_io_channel_set_encoding(gio, NULL, NULL);

		for (i=0;i<argc;i++) {
			if (strncmp(argv[i],"--working-directory=", 20)==0) {
				setworkdir = TRUE;
			}

			g_io_channel_write_chars(gio, g_shell_quote(*(argv+i)), -1, NULL, NULL);
			if (i+1!=argc) {
				g_io_channel_write_chars(gio, " ", -1, NULL, NULL);
			} else {
				if (!setworkdir) {
					gchar *workdir = g_get_current_dir();
					g_io_channel_write_chars(gio, " --working-directory=", -1, NULL, NULL);
					g_io_channel_write_chars(gio, workdir, -1, NULL, NULL);
					g_free(workdir);
				}
			}
		}

		g_io_channel_write_chars(gio, "\n", -1, NULL, NULL);
		g_io_channel_flush(gio, NULL);
		g_io_channel_unref(gio);
		g_free(socket_path);
		return FALSE;
	}
}
