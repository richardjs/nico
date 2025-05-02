#!/usr/bin/env python3

import argparse
import time
from subprocess import run

import requests


ENGINE = "./nico_wrapper.py"


def engine(args) -> (str, str):
    print(f"running {[ENGINE] + args}")
    p = run([ENGINE] + args, capture_output=True, encoding="utf-8")

    if p.returncode:
        raise HTTPException(status_code=422, detail=p.stderr)

    return p.stdout, p.stderr


parser = argparse.ArgumentParser()
parser.add_argument("-p", "--player")
parser.add_argument("-t", "--token")
parser.add_argument("-e", "--event")
parser.add_argument("-H", "--host", default="https://softserve.harding.edu")
parser.add_argument("nico_flags", nargs="+")
args = parser.parse_args()


while 1:
    print("requesting state...")
    r = requests.post(
        f"{args.host}/aivai/play-state",
        json={
            "player": args.player,
            "token": args.token,
            "event": args.event,
        },
    )
    if r.status_code == 204:
        print("no games")
        time.sleep(1)
        continue
    elif r.status_code != 200:
        print(r.content)
        break

    d = r.json()

    print(f"got state {d['state']}")
    action, stderr = engine(args.nico_flags + [d["state"]])

    print(f"sending action: {action}")
    d = {
        "player": args.player,
        "token": args.token,
        "action_id": d["action_id"],
        "action": action.strip(),
    }
    r = requests.post(f"{args.host}/aivai/submit-action", json=d)

    if r.status_code != 200:
        print(r.content)
        break
