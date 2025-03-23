#!/usr/bin/env python3


import argparse
import os
import re
import sys
from pathlib import Path
from subprocess import run, PIPE


class State:
    def __init__(self, string):
        self.tiles = [
            (int(s.split(",")[0]), int(s.split(",")[1]))
            for s in re.findall(r"(-?\d?\d,-?\d?\d)\|", string)
        ]

        self.stacks = []
        for stack_string in re.findall(r"(-?\d?\d,-?\d?\d[ht]\d?\d)\|", string):
            coord, count = re.split("[ht]", stack_string)
            q, r = coord.split(",")
            q = int(q)
            r = int(r)
            count = int(count)
            player = re.findall("[ht]", stack_string)[0]

            self.stacks.append((q, r, player, count))

        self.turn = string[-1]

        self.offset = (0, 0)

    @property
    def inverse_offset(self):
        q, r = self.offset
        return -q, -r

    def translate(self, vector):
        if not self.tiles:
            return

        tq, tr = vector
        self.tiles = [(q + tq, r + tr) for (q, r) in self.tiles]
        self.stacks = [(q + tq, r + tr, p, c) for q, r, p, c in self.stacks]

        q, r = self.offset
        self.offset = q + tq, r + tr

    def denormalize(self):
        self.translate(self.inverse_offsetoffset)

    def normalize(self):
        if not self.tiles:
            return

        min_q = min([tile[0] for tile in self.tiles])
        min_r = min([tile[1] for tile in self.tiles])
        self.translate((-min_q, -min_r))

    def __str__(self):
        s = ""
        for tile in self.tiles:
            q, r = tile
            s += f"{q},{r}|"

        for stack in self.stacks:
            q, r, p, c = stack
            s += f"{q},{r}{p}{c}|"

        s += self.turn

        return s


class Action:
    def __init__(self, string):
        m = re.match(
            r"(-?\d+),(-?\d+)\|(-?\d+),(-?\d+)\|(-?\d+),(-?\d+)\|(-?\d+),(-?\d+)",
            string,
        )
        if m:
            q1, r1, q2, r2, q3, r3, q4, r4 = m.groups()
            self.coords = [
                (int(q1), int(r1)),
                (int(q2), int(r2)),
                (int(q3), int(r3)),
                (int(q4), int(r4)),
            ]
            return

        m = re.match(r"(-?\d+),(-?\d+)\|(\d+)\|(-?\d+),(-?\d+)", string)
        if m:
            q1, r1, stack, q2, r2 = m.groups()
            self.coords = [
                (int(q1), int(r1)),
                (int(q2), int(r2)),
            ]
            self.stack = int(stack)
            return

        m = re.match(r"(-?\d+),(-?\d+)", string)
        if m:
            q1, r1 = m.groups()
            self.coords = [(int(q1), int(r1))]
            return

        raise Exception(f"Cannot parse action string {string}")

    def translate(self, vector):
        tq, tr = vector
        self.coords = [(q + tq, r + tr) for q, r in self.coords]

    def __str__(self):
        if len(self.coords) == 4:
            (q1, r1), (q2, r2), (q3, r3), (q4, r4) = self.coords
            return f"{q1},{r1}|{q2},{r2}|{q3},{r3}|{q4},{r4}"

        if len(self.coords) == 2:
            (q1, r1), (q2, r2) = self.coords
            return f"{q1},{r1}|{self.stack}|{q2},{r2}"

        if len(self.coords) == 1:
            ((q1, r1),) = self.coords
            return f"{q1},{r1}"


def main():
    sys.stderr.write(" ".join(sys.argv) + "\n")

    # Use a different prefix to avoid confusion with negative numbers
    parser = argparse.ArgumentParser(prefix_chars="/")

    parser.add_argument("state", nargs="?", default="")

    parser.add_argument("/a", "//act")
    parser.add_argument("/i", "//iterations", type=int)
    parser.add_argument("/w", "//workers", type=int)

    parser.add_argument("/I", "//initial", action="store_true")
    parser.add_argument("/l", "//list-actions", action="store_true")
    parser.add_argument("/n", "//normalize", action="store_true")
    parser.add_argument("/P", "//print", action="store_true")
    parser.add_argument("/r", "//random", action="store_true")
    parser.add_argument("/t", "//think", action="store_true")
    parser.add_argument("/v", "//version", action="store_true")
    parser.add_argument("/W", "//winner", action="store_true")

    args = parser.parse_args()

    invocation = [str(Path(__file__).resolve().parent / "nico")]

    if args.initial:
        invocation.append("-I")
    if args.list_actions:
        invocation.append("-l")
    if args.normalize:
        invocation.append("-n")
    if args.print:
        invocation.append("-P")
    if args.random:
        invocation.append("-r")
    if args.think:
        invocation.append("-t")
    if args.version:
        invocation.append("-v")
    if args.winner:
        invocation.append("-W")

    if args.iterations:
        invocation += ["-i", args.iterations]

    if args.state:
        state = State(args.state)
        state.normalize()

    if args.act:
        action = Action(args.act)
        action.translate(state.offset)
        invocation += ["-a", str(action)]

    if args.state:
        invocation.append(str(state))

    sys.stderr.write(f"wrapped invocation: {invocation}\n")
    sys.stderr.flush()
    p = run(invocation, stdout=PIPE)

    stdout = p.stdout.decode("utf-8")
    sys.stderr.write(f"wrapped stdout: {stdout}\n")

    if args.initial or args.winner:
        sys.stdout.write(stdout)

    elif args.list_actions:
        if stdout.strip() == "terminal state":
            sys.stdout.write(stdout)
        else:
            for line in stdout.split():
                action = Action(line)
                action.translate(state.inverse_offset)
                print(str(action))

    elif args.think or args.random:
        action = Action(stdout.strip())
        action.translate(state.inverse_offset)
        print(str(action))

    elif args.act:
        after_state = State(stdout.strip())
        after_state.translate(state.inverse_offset)
        print(str(after_state))


if __name__ == "__main__":
    main()
