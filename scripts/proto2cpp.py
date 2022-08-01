import argparse
import sys, pathlib
import collections

# We have to do this because import paths in generated python proto files follow
# the paths of the original .proto sources, which means that they are are
# relative to the root of the proto schema directory. By putting the generated
# filed in the "vlsir" subdirectory, we break them. We could dump them in the
# same folder as this script, or we could hack it like in
# https://github.com/growly/Vlsir/blob/main/scripts/primitives.py

here = pathlib.Path(__file__).parent.absolute()
pybindings = here / "vlsir"
sys.path.append(str(pybindings))

from vlsir.layout import raw_pb2
from vlsir import tech_pb2


def name_from_layer_info(layer_info: tech_pb2.LayerInfo) -> str:
    name = layer_info.name
    purpose = layer_info.purpose
    if purpose.description:
        name += f'.{layer_info.purpose.description}'
    return name


def comment_from_layer_info(layer_info: tech_pb2.LayerInfo) -> str:
    comment = name_from_layer_info(layer_info)
    purpose = layer_info.purpose
    if purpose.type != (
        tech_pb2.LayerPurposeType.UNKNOWN):
        comment += f' [{tech_pb2.LayerPurposeType.Name(purpose.type)}]'
    comment += f' {layer_info.index}/{layer_info.sub_index}'
    return comment


def main():
    parser = argparse.ArgumentParser(description='no description')
    parser.add_argument('--layout', '-i', default='', required=True)
    parser.add_argument('--tech', '-t', default='', required=False)
    args = parser.parse_args()

    library = raw_pb2.Library()
    with open(args.layout, 'rb') as f:
        library.ParseFromString(f.read())

    if library.units != raw_pb2.Units.NANO:
        raise NotImplementedError(
            'Currently assume all numbers in the input are nanometres')

    # Look up layer names by layer_infos[index][sub_index].
    layer_infos = collections.defaultdict(dict)
    if args.tech:
        tech = tech_pb2.Technology()
        with open(args.tech, 'rb') as f:
            tech.ParseFromString(f.read())
        for layer_info in tech.layers:
            layer_infos[layer_info.index][layer_info.sub_index] = layer_info

    add_polygon_start = 'layout->AddPolygon(Polygon({'
    indents = ' ' * len(add_polygon_start)

    for cell in library.cells:
        layout = cell.layout
        
        # Instances.
        if layout.instances:
            raise NotImplementedError(
                f'this cell has {len(layout.instances)} instances!')

        # Shapes per layer.
        for shapes in layout.shapes:
            index = shapes.layer.number
            sub_index = shapes.layer.purpose
            try:
                layer_info = layer_infos[index][sub_index]
            except KeyError:
                layer_info = None

            if layer_info is not None:
                print(f'// {comment_from_layer_info(layer_info)}')
                print('layout->SetActiveLayerByName("'
                      f'{name_from_layer_info(layer_info)}");')

            for rect_pb in shapes.rectangles:
                ll = rect_pb.lower_left
                ur = (ll.x + rect_pb.width, ll.y + rect_pb.height)
                print(f'layout->AddRectangle(Rectangle(Point({ll.x}, {ll.y}), Point({ur[0]}, {ur[1]})));')

            for poly_pb in shapes.polygons:
                point_list = f',\n{indents}'.join(
                    f'Point({vertex.x}, {vertex.y})' for vertex in
                    poly_pb.vertices)
                print(f'{add_polygon_start}{point_list}}}));')

            if shapes.paths:
                raise NotImplementedError(
                    f'this layer has {len(shapes.paths)} paths!')

            print('')

        # Annotations.


if __name__ == '__main__':
    main()
