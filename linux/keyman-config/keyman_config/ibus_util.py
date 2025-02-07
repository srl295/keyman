#!/usr/bin/python3

import os
import time
import gi
import logging
import re
import subprocess
from keyman_config.gnome_keyboards_util import is_gnome_shell

from keyman_config.gsettings import GSettings

gi.require_version('IBus', '1.0')
from gi.repository import IBus


class IbusUtil():
    def __init__(self) -> None:
        self.ibus_settings = GSettings('org.freedesktop.ibus.general')

    def read_preload_engines(self):
        return self.ibus_settings.get('preload-engines')

    def write_preload_engines(self, bus, preload_engines):
        self.ibus_settings.set('preload-engines', preload_engines, 'as')
        if bus:
            bus.preload_engines(preload_engines)


def get_ibus_bus():
    try:
        for i in range(5):
            bus = IBus.Bus()
            if bus.is_connected() and bus.is_global_engine_enabled():
                return bus
            bus.destroy()
            time.sleep(1)
    except Exception as e:
        logging.warning("Failed get bus")
        logging.warning(e)
    logging.warning("could not find connected IBus.Bus")
    return None


def install_to_ibus(bus, keyboard_id):
    try:
        logging.debug("installing to ibus")
        ibusUtil = IbusUtil()
        preload_engines = ibusUtil.read_preload_engines()
        logging.debug('preload_engines before: %s', preload_engines)
        if keyboard_id not in preload_engines:
            # TODO: in the event preload_engines contains upper-case keyboards, we'll need to uninstall_from_ibus #1601
            preload_engines.append(keyboard_id)
        logging.debug('Setting preload_engines to: %s', preload_engines)
        ibusUtil.write_preload_engines(bus, preload_engines)
    except Exception as e:
        logging.warning("Failed to set up install %s to IBus", keyboard_id)
        logging.warning(e)


def uninstall_from_ibus(bus, keyboard_id):
    # need to uninstall for all installed langs
    try:
        logging.debug('Uninstalling from ibus')
        ibusUtil = IbusUtil()
        preload_engines = ibusUtil.read_preload_engines()
        logging.debug('preload_engines before: %s', preload_engines)
        if keyboard_id in preload_engines:
            preload_engines.remove(keyboard_id)
        logging.debug('Setting preload_engines to: %s', preload_engines)
        ibusUtil.write_preload_engines(bus, preload_engines)
    except Exception as e:
        logging.warning("Failed to uninstall keyboard %s", keyboard_id)
        logging.warning(e)


def restart_ibus_subp():
    logging.info("restarting IBus by subprocess")
    subprocess.run(["ibus", "restart"])


def _verify_ibus_daemon():
    realuser = os.environ.get('SUDO_USER')
    user = os.environ.get('USER')
    if realuser:
        user = realuser
    elif not user:
        user = os.environ.get('LOGNAME')

    try:
        ps = subprocess.run(('ps', '--user', user, '-o', 's=', '-o', 'cmd'), stdout=subprocess.PIPE).stdout
        if not re.search('^[^ZT] ibus-daemon .*--xim.*', ps.decode('utf-8'), re.MULTILINE):
            _start_ibus_daemon(realuser)
    except subprocess.CalledProcessError:
        _start_ibus_daemon(realuser)


def _start_ibus_daemon(realuser):
    try:
        args = ['ibus-daemon', '-d', '-r', '--xim']
        if is_gnome_shell():
            # on Ubuntu 21.10 with Gnome the keyboards don't show in dropdown list if we don't disable the panel
            args.extend(['--panel', 'disable'])

        if realuser:
            # we have been called with `sudo`. Start ibus-daemon for the real user.
            logging.info('starting ibus-daemon for user %s', realuser)
            subprocess.run(['sudo', '-u', realuser].extend(args))
        else:
            logging.info('ibus-daemon not running. Starting it...')
            subprocess.run(args)
    except Exception:
        logging.warning('Failed to start ibus-daemon')


def restart_ibus(bus=None):
    realuser = os.environ.get('SUDO_USER')
    if realuser:
        # we have been called with `sudo`. Restart ibus for the real user.
        logging.info('restarting IBus by subprocess for user %s', realuser)
        subprocess.run(['sudo', '-u', realuser, 'ibus', 'restart'])
    else:
        try:
            if not bus:
                bus = get_ibus_bus()
            if bus:
                logging.info("restarting IBus")
                bus.exit(True)
                bus.destroy()
        except Exception as e:
            logging.warning("Failed to restart IBus")
            logging.warning(e)
    _verify_ibus_daemon()


def bus_has_engine(bus, name):
    engines = bus.get_engines_by_names([name])
    return len(engines)


def get_current_engine(bus):
    try:
        contextname = bus.current_input_context()
        ic = IBus.InputContext.get_input_context(contextname, bus.get_connection())
        engine = ic.get_engine()
        if engine:
            return engine.get_name()
    except Exception as e:
        logging.warning("Failed to get current engine")
        logging.warning(e)


def change_to_keyboard(bus, keyboard_id):
    try:
        contextname = bus.current_input_context()
        ic = IBus.InputContext.get_input_context(contextname, bus.get_connection())
        if bus_has_engine(bus, keyboard_id) <= 0:
            logging.warning("Could not find engine %s" % keyboard_id)
        else:
            ic.set_engine(keyboard_id)
    except Exception as e:
        logging.warning("Failed to change keyboard")
        logging.warning(e)
