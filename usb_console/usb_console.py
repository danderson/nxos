#!/usr/bin/env python

import sys
import os.path
import threading
import Queue
import curses
import curses.wrapper
import time;
from nxt.lowlevel import get_device

NXOS_INTERFACE = 0

COMMAND = 0
INPUT = 1
OUTPUT = 2

def usb_thread(brick, command_queue, output_queue):
    output_queue.put((COMMAND, "-- USB I/O thread up"))
    while True:
        from_brick = brick.read(128)
        if from_brick:
            output_queue.put((INPUT, "<< %s" % repr(from_brick)[1:-1]))
        try:
            to_brick = command_queue.get_nowait()
            if to_brick is None:
                brick.close()
                return
            while not (brick.write(to_brick)):
                time.sleep(1)
        except Queue.Empty:
            pass

def output_thread(stdscr, output_queue):
    curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLACK)
    curses.init_pair(2, curses.COLOR_RED, curses.COLOR_BLACK)
    curses.init_pair(3, curses.COLOR_YELLOW, curses.COLOR_BLACK)

    y,x = stdscr.getmaxyx()
    logpos = 1
    log = curses.newwin(y-1,x,0,0)
    log.border()
    log.noutrefresh()
    stdscr.noutrefresh()
    curses.doupdate()
    output_queue.put((COMMAND, "-- Output thread up"))
    while True:
        data = output_queue.get()
        if data is None:
            return
        log.addstr(logpos, 1, repr(data[1])[1:-1], curses.color_pair(data[0]+1))
        #stdscr.move(y-1,0)
        log.noutrefresh()
        stdscr.noutrefresh()
        curses.doupdate()
        logpos += 1
        if logpos == y:
            logpos=1

def init(stdscr):
    brick = get_device(0x0694, 0xff00, timeout=60)
    if not brick:
        return False
    brick.open(NXOS_INTERFACE)

    command_queue = Queue.Queue(1)
    output_queue = Queue.Queue()

    output = threading.Thread(target=output_thread, args=[stdscr, output_queue])
    output.start()

    usb = threading.Thread(target=usb_thread,
                           args=[brick, command_queue, output_queue])
    usb.start()

    return (command_queue, output_queue, output, usb)

def input_loop(stdscr, command_queue, output_queue):
    try:
        curses.echo()
        while True:
            mx = stdscr.getmaxyx()
            send = stdscr.getstr(mx[0]-1,0).strip()
            stdscr.clrtoeol()
            output_queue.put((OUTPUT, '>> %s' % send))
            command_queue.put(send)
            if send == 'halt':
                break
    except KeyboardInterrupt:
        pass

def ncurses_main(stdscr):
    ret = init(stdscr)
    if not ret:
        return 1
    command_queue, output_queue, output, usb = ret

    input_loop(stdscr, command_queue, output_queue)

    command_queue.put(None)
    output_queue.put(None)
    output.join()
    usb.join()

def main():
    curses.wrapper(ncurses_main)

if __name__ == '__main__':
    sys.exit(main())
