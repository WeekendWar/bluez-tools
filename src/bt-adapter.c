/*
 *
 *  bluez-tools - a set of tools to manage bluetooth devices for linux
 *
 *  Copyright (C) 2010  Alexander Orlenko <zxteam@gmail.com>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <glib.h>

#include "lib/dbus-common.h"
#include "lib/helpers.h"
#include "lib/adapter.h"
#include "lib/device.h"
#include "lib/manager.h"

#define DISCOVERY_TIMEOUT 30

static gboolean stop_discovery(gpointer data)
{
	GMainLoop *mainloop = data;
	g_main_loop_quit(mainloop);
	return FALSE;
}

static void adapter_device_found(Adapter *adapter, const gchar *address, GHashTable *values, gpointer data)
{
	GHashTable *found_devices = data;

	if (g_hash_table_lookup(found_devices, address) != NULL) {
		return;
	}

	if (g_hash_table_size(found_devices) == 0) g_print("\n");

	g_print("[%s]\n", address);
	g_print("  Name: %s\n", g_value_get_string(g_hash_table_lookup(values, "Name")));
	g_print("  Alias: %s\n", g_value_get_string(g_hash_table_lookup(values, "Alias")));
	g_print("  Address: %s\n", g_value_get_string(g_hash_table_lookup(values, "Address")));
	g_print("  Class: %d\n", g_value_get_uint(g_hash_table_lookup(values, "Class")));
	g_print("  LegacyPairing: %d\n", g_value_get_boolean(g_hash_table_lookup(values, "LegacyPairing")));
	g_print("  Paired: %d\n", g_value_get_boolean(g_hash_table_lookup(values, "Paired")));
	g_print("  RSSI: %d\n", g_value_get_int(g_hash_table_lookup(values, "RSSI")));
	g_print("\n");

	g_hash_table_insert(found_devices, g_strdup(address), g_strdup(".."));
}

static void adapter_device_disappeared(Adapter *adapter, const gchar *address, gpointer data)
{
	//GHashTable *found_devices = data;
	//g_print("Device disappeared: %s\n", address);
}

static gboolean list_arg = FALSE;
static gchar *adapter_arg = NULL;
static gboolean info_arg = FALSE;
static gboolean discover_arg = FALSE;
static gboolean set_arg = FALSE;
static gchar *set_name_arg = NULL;
static gchar *set_value_arg = NULL;

static GOptionEntry entries[] = {
	{ "list", 'l', 0, G_OPTION_ARG_NONE, &list_arg, "List all available adapters", NULL},
	{ "adapter", 'a', 0, G_OPTION_ARG_STRING, &adapter_arg, "Adapter name or MAC", "adapter#id"},
	{ "info", 'i', 0, G_OPTION_ARG_NONE, &info_arg, "Show adapter info", NULL},
	{ "discover", 'd', 0, G_OPTION_ARG_NONE, &discover_arg, "Discover remote devices", NULL},
	{ "set", 0, 0, G_OPTION_ARG_NONE, &set_arg, "Set property", NULL},
	{ NULL}
};

int main(int argc, char *argv[])
{
	GError *error = NULL;
	GOptionContext *context;

	g_type_init();

	context = g_option_context_new("[--set Name Value] - a bluetooth adapter manager");
	g_option_context_add_main_entries(context, entries, NULL);
	g_option_context_set_summary(context, "summary");
	g_option_context_set_description(context, "desc");

	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_print("%s: %s\n", g_get_prgname(), error->message);
		g_print("Try `%s --help` for more information.\n", g_get_prgname());
		exit(EXIT_FAILURE);
	} else if (!list_arg && !info_arg && !discover_arg && !set_arg) {
		g_print("%s", g_option_context_get_help(context, FALSE, NULL));
		exit(EXIT_FAILURE);
	} else if (set_arg && argc != 3) {
		g_print("%s: Invalid arguments for --set\n", g_get_prgname());
		g_print("Try `%s --help` for more information.\n", g_get_prgname());
		exit(EXIT_FAILURE);
	}

	g_option_context_free(context);

	if (!dbus_connect(&error)) {
		g_printerr("Couldn't connect to dbus: %s", error->message);
		exit(EXIT_FAILURE);
	}

	Manager *manager = g_object_new(MANAGER_TYPE, NULL);

	if (list_arg) {
		const GPtrArray *adapters_list = manager_get_adapters(manager);
		g_assert(adapters_list != NULL);

		g_print("Available adapters:\n");
		if (adapters_list->len == 0) {
			g_print("no adapters found\n");
		}

		for (int i = 0; i < adapters_list->len; i++) {
			const gchar *adapter_path = g_ptr_array_index(adapters_list, i);
			Adapter *adapter = g_object_new(ADAPTER_TYPE, "DBusObjectPath", adapter_path, NULL);
			g_print("%s (%s)\n", adapter_get_name(adapter), adapter_get_address(adapter));
			g_object_unref(adapter);
		}
	} else if (info_arg) {
		Adapter *adapter = find_adapter(adapter_arg, &error);
		exit_if_error(error);

		g_print("[%s]\n", g_basename(adapter_get_dbus_object_path(adapter)));
		g_print("  Name: %s [rw]\n", adapter_get_name(adapter));
		g_print("  Address: %s\n", adapter_get_address(adapter));
		g_print("  Class: 0x%x\n", adapter_get_class(adapter));
		g_print("  Discoverable: %d [rw]\n", adapter_get_discoverable(adapter));
		g_print("  DiscoverableTimeout: %d [rw]\n", adapter_get_discoverable_timeout(adapter));
		g_print("  Discovering: %d\n", adapter_get_discovering(adapter));
		g_print("  Pairable: %d [rw]\n", adapter_get_pairable(adapter));
		g_print("  PairableTimeout: %d [rw]\n", adapter_get_pairable_timeout(adapter));
		g_print("  Powered: %d [rw]\n", adapter_get_powered(adapter));
		g_print("  Service(s): [");
		const gchar **uuids = adapter_get_uuids(adapter);
		for (int j = 0; uuids[j] != NULL; j++) {
			if (j > 0) g_print(", ");
			g_print("%s", uuid2service(uuids[j]));
		}
		g_print("]\n");

		g_object_unref(adapter);
	} else if (discover_arg) {
		Adapter *adapter = find_adapter(adapter_arg, &error);
		exit_if_error(error);

		// To store pairs MAC => Name
		GHashTable *found_devices = g_hash_table_new(g_str_hash, g_str_equal);

		g_signal_connect(adapter, "DeviceFound", G_CALLBACK(adapter_device_found), found_devices);
		g_signal_connect(adapter, "DeviceDisappeared", G_CALLBACK(adapter_device_disappeared), found_devices);

		adapter_start_discovery(adapter, &error);
		exit_if_error(error);

		g_print("Searching...\n");

		GSource *timeout_src = g_timeout_source_new_seconds(DISCOVERY_TIMEOUT);
		g_source_attach(timeout_src, NULL);
		GMainLoop *mainloop = g_main_loop_new(NULL, FALSE);
		g_source_set_callback(timeout_src, stop_discovery, mainloop, NULL);
		g_main_loop_run(mainloop);

		/* Discovering process here... */

		g_main_loop_unref(mainloop);
		g_source_unref(timeout_src);

		adapter_stop_discovery(adapter, &error);
		exit_if_error(error);

		g_print("Done\n");

		g_hash_table_unref(found_devices);
		g_object_unref(adapter);
	} else if (set_arg) {
		Adapter *adapter = find_adapter(adapter_arg, &error);
		exit_if_error(error);

		set_name_arg = argv[1];
		set_value_arg = argv[2];

		GValue v = {0,};

		if (g_strcmp0(set_name_arg, "Name") == 0) {
			g_value_init(&v, G_TYPE_STRING);
			g_value_set_string(&v, set_value_arg);
		} else if (
				g_strcmp0(set_name_arg, "Discoverable") == 0 ||
				g_strcmp0(set_name_arg, "Pairable") == 0 ||
				g_strcmp0(set_name_arg, "Powered") == 0
				) {
			g_value_init(&v, G_TYPE_BOOLEAN);

			if (g_strcmp0(set_value_arg, "0") == 0 || g_strcmp0(set_value_arg, "FALSE") == 0) {
				g_value_set_boolean(&v, FALSE);
			} else if (g_strcmp0(set_value_arg, "1") == 0 || g_strcmp0(set_value_arg, "TRUE") == 0) {
				g_value_set_boolean(&v, TRUE);
			} else {
				g_print("Invalid value: %s\n", set_value_arg);
			}
		} else if (
				g_strcmp0(set_name_arg, "DiscoverableTimeout") == 0 ||
				g_strcmp0(set_name_arg, "PairableTimeout") == 0
				) {
			g_value_init(&v, G_TYPE_UINT);
			g_value_set_uint(&v, (guint) atoi(set_value_arg));
		} else {
			g_print("Invalid property: %s\n", set_name_arg);
			exit(EXIT_FAILURE);
		}

		adapter_set_property(adapter, set_name_arg, &v, &error);
		exit_if_error(error);

		g_print("Done\n");

		g_object_unref(adapter);
	}

	g_object_unref(manager);

	exit(EXIT_SUCCESS);
}
