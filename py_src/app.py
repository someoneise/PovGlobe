import flask
from flask import request, redirect, render_template

import threading

from globe_wrapper import GlobeWrapper

app = flask.Flask(__name__)

globe_wrapper = GlobeWrapper()
sem = threading.Semaphore()


@app.route("/", methods=["GET"])
def home():
    res = render_template(
        "app_select.html",
        app_names=globe_wrapper.get_all_apps().keys(),
        running_app_name=globe_wrapper.running_app_name,
    )
    return res


@app.route("/launch_app", methods=["GET", "POST"])
def launch_app():

    app_name = request.args.get("name", default=None, type=str)

    apps = globe_wrapper.get_all_apps()
    if app_name is None or app_name not in apps:
        return "Invalid App Name"

    app = apps[app_name]

    args = []
    for arg in app["args"]:
        arg_name = arg["name"]
        arg_val = None
        if "options" in arg:
            for o in arg["options"]:
                arg_val = request.args.get(f"{arg_name}_{o}", default=None, type=str)
                if arg_val == "on":
                    arg_val = arg["options"][o]
                    break

        args.append(arg_val)

    sem.acquire()
    success = globe_wrapper.app_by_name(app_name, args)
    sem.release()

    if success:
        return redirect("/", code=302)
    else:
        return "Failed"


@app.route("/configure_app", methods=["GET"])
def configure_app():
    app_name = request.args.get("name", default=None, type=str)

    apps = globe_wrapper.get_all_apps()

    if app_name is None or app_name not in apps:
        return "Invalid App Name"

    app = apps[app_name]

    simple_args = []
    options_args = []

    for arg in app["args"]:
        if arg["type"] == "img_path":
            options_args.append(arg)
        elif arg["type"] == "proj":
            options_args.append(arg)
        elif arg["type"] == "interp":
            options_args.append(arg)

    res = render_template(
        "configure_app.html",
        app_name=app_name,
        simple_args=simple_args,
        options_args=options_args,
    )
    return res


if __name__ == "__main__":
    app.run(host="0.0.0.0", processes=1)
