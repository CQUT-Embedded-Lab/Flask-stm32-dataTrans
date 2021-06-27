from flask import Flask, render_template, request

from client import Client

app = Flask(__name__)
client = Client()


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/connect", methods=["POST"])
def connect():
    global client
    ipaddr = request.form.get("ipaddr")
    try:
        client.connect(ipaddr)
        return {'message': "success!"}
    except:
        return {'message': "not good"}


@app.route("/deconnect", methods=["POST"])
def deconnect():
    global client
    ipaddr = request.form.get("ipaddr")
    client.deconnect()
    return {'message': "success!"}


@app.route("/send_msg", methods=["POST"])
def send_msg():
    global client
    msg = request.form.get("msg")
    client.send_msg(msg)
    return {'message': "success!", 'msg': msg}


@app.route("/send_on", methods=["POST"])
def send_on():
    msg = request.form.get("data")
    client.send_msg(msg)
    return {'message': "success!", 'msg': msg}


@app.route("/send_off", methods=["POST"])
def send_off():
    msg = request.form.get("data")
    client.send_msg(msg)
    return {'message': "success!", 'msg': msg}


if __name__ == '__main__':
    app.run()
