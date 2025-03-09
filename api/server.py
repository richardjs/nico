import re
from os import environ
from subprocess import run
from typing import Annotated

from fastapi import FastAPI, Header, HTTPException, Path
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


app = FastAPI()


class EngineResponse(BaseModel):
    log: str


class ThinkResponse(EngineResponse):
    action: str


def engine(*args) -> (str, str):
    p = run([ENGINE] + list(args), capture_output=True, encoding="utf-8")

    if p.returncode:
        raise HTTPException(status_code=422, detail=p.stderr)

    return p.stdout, p.stderr


def get_actions(state: str) -> (list[str], str):
    stdout, stderr = engine("-l", state)

    if stdout.strip() == "terminal state":
        return ([], stderr)

    return (stdout.strip().split("\n"), stderr)


@app.get("/state/{state}/think", response_model=ThinkResponse)
async def state_think(
    state: str,  # TODO = Path(pattern=STATE_REGEX),
    workers: Annotated[str | None, Header()] = None,
    iterations: Annotated[str | None, Header()] = None,
) -> ThinkResponse:

    if workers:
        workers = min(int(workers), MAX_WORKERS)
        workers = max(workers, MIN_WORKERS)

    if iterations:
        iterations = min(int(iterations), MAX_ITERATIONS)
        iterations = max(iterations, MIN_ITERATIONS)

    # action, stderr = engine(f"-t", "-w", str(workers), "-i", str(iterations), str(state))
    action, stderr = engine(f"-t", str(state))

    return ThinkResponse(action=str(action), log=stderr)
