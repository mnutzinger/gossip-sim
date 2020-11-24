import argparse
import imageio
import json
from networkx.drawing.nx_pydot import to_pydot
from networkx.readwrite.json_graph import node_link_graph
import os
import pydot
import sys
from tempfile import mkstemp
from typing import List


def set_common_attributes(graph: pydot.Dot) -> None:
    graph.set_layout("neato")
    graph.set_overlap("false")
    graph.set_splines("compound")
    graph.set_sep("+10")

    for node in graph.get_nodes():
        node.set_shape("circle")
        node.set_style("filled")
        node.set_height("0.15")
        node.set_fontsize("5")
        node.set_fixedsize("true")
        node.set_color("#5aa469")

    for edge in graph.get_edges():
        edge.set_penwidth("0.3")


def generate_image(graph: pydot.Dot, red_nodes: List[str]) -> str:
    for node in red_nodes:
        node = graph.get_node(node)[0]
        node.set_color("#d35d6e")

    _, png = mkstemp()
    with open(png, "wb") as f:
        f.write(graph.create_png())
    print(f"Wrote intermediate step to {png}")
    return png


def generate_gif(json_results: str, gif: str) -> str:
    with open(json_results) as f:
        data = json.load(f)

    graph = to_pydot(node_link_graph(data))
    set_common_attributes(graph)

    iterations = max([node["iterations"] for node in data["nodes"]])
    images = []

    for it in range(0, iterations+1):
        red_nodes = [node["id"] for node in data["nodes"] if node["iterations"] == it]
        images.append(generate_image(graph, red_nodes))

    imageio.mimwrite(gif, [imageio.imread(image) for image in images], duration=1)
    print(f"Wrote gif to {gif}")

    for image in images:
        os.unlink(image)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--json",
                        type=str,
                        required=True,
                        help="Path to Json results generated from gossip simulator")
    parser.add_argument("--gif",
                        type=str,
                        required=True,
                        help="Path to store generated gif")
    args = parser.parse_args()

    generate_gif(args.json, args.gif)

