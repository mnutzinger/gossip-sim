import argparse
import socket
import sys


MESSAGE: str = "Hello, world!"


def inject_message(*port: int) -> None:
    sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)

    for p in port:
        addr = ("::1", p)
        num = sock.sendto(str.encode(MESSAGE), addr)
        print(f"Wrote {num} bytes to :{p}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--port",
                        type=int,
                        required=True,
                        nargs="+",
                        help="Port number of gossip node")
    args = parser.parse_args()

    inject_message(*args.port)

