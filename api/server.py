import re
from os import environ
from subprocess import run
from typing import Annotated

from fastapi import FastAPI, Header, Path
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel


WORKERS = 4
ITERATIONS = 1000

MAX_ITERATIONS = 50000
MIN_ITERATIONS = 100

MAX_WORKERS = 4
MIN_WORKERS = 1

ENGINE = environ.get("ENGINE")
if not ENGINE:
    raise Exception("No engine defined!")


STATE_REGEX = r"^(-?\d+,-?\d+\|){0,32}(-?\d+,-?\d+[ht]\d+|){0,32}[ht]$"


class State:
    def __init__(self, string):
        tile_strings = re.findall(r"(-?\d?\d,-?\d?\d)\|", string)
        self.tiles = [
            (int(s.split(",")[0]), int(s.split(",")[1])) for s in tile_strings
        ]

        self.stacks = []
        stack_strings = re.findall(r"(-?\d?\d,-?\d?\d[ht]\d?\d)\|", string)
        for stack_string in stack_strings:
            coord, count = re.split("[ht]", stack_string)
            q, r = coord.split(",")
            q = int(q)
            r = int(r)
            count = int(count)
            player = re.findall("[ht]", stack_string)[0]

            self.stacks.append((q, r, player, count))

        self.turn = string[-1]

        self.untranslation = (0, 0)

    def translate(self, vector):
        if not self.tiles:
            return

        tq, tr = vector
        self.tiles = [(q + tq, r + tr) for (q, r) in self.tiles]

        stacks = self.stacks
        self.stacks = []
        for q, r, p, c in stacks:
            self.stacks.append((q + tq, r + tr, p, c))

        q, r = self.untranslation
        self.untranslation = q - tq, r - tr

    def untranslate(self):
        self.translate(self.untranslation)

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


app = FastAPI()


class EngineResponse(BaseModel):
    log: str


class ThinkResponse(EngineResponse):
    action: str


def engine(*args) -> (str, str):
    p = run([ENGINE] + list(args), capture_output=True, encoding="utf-8")

    if p.returncode:
        raise Exception(status_code=422, detail=p.stderr)

    return p.stdout, p.stderr


def get_actions(state: str) -> (list[str], str):
    stdout, stderr = engine("-l", state)

    if stdout.strip() == "terminal state":
        return ([], stderr)

    return (stdout.strip().split("\n"), stderr)


@app.get("/state/{state}/think", response_model=ThinkResponse)
async def state_think(
    state: str,  # = Path(pattern=STATE_REGEX),
    workers: Annotated[str | None, Header()] = None,
    iterations: Annotated[str | None, Header()] = None,
) -> ThinkResponse:

    if workers:
        workers = min(int(workers), MAX_WORKERS)
        workers = max(workers, MIN_WORKERS)

    if iterations:
        iterations = min(int(iterations), MAX_ITERATIONS)
        iterations = max(iterations, MIN_ITERATIONS)

    state = State(state)
    state.normalize()

    # action, stderr = engine(f"-t", "-w", str(workers), "-i", str(iterations), str(state))
    action, stderr = engine(f"-t", str(state))

    action = Action(action.strip())
    action.translate(state.untranslation)
    print(stderr)

    return ThinkResponse(action=str(action), log=stderr)
