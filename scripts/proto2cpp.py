import argparse
import sys, pathlib
import collections
from collections.abc import Sequence
from typing import Any, Optional

# We have to do this because import paths in generated python proto files follow
# the paths of the original .proto sources, which means that they are are
# relative to the root of the proto schema directory. By putting the generated
# files in the "vlsir" subdirectory, we break them. We could dump them in the
# same folder as this script, or we could hack it like in
# https://github.com/growly/Vlsir/blob/main/scripts/primitives.py (which is what
# we do).

here = pathlib.Path(__file__).parent.absolute()
pybindings = here / "vlsir"
sys.path.append(str(pybindings))

from vlsir.layout import raw_pb2
from vlsir import tech_pb2


class Point:

    def __init__(self, point_pb: raw_pb2.Point = None):
        self.x = 0
        self.y = 0
        if point_pb is not None:
            self.x = point_pb.x
            self.y = point_pb.y

    def __eq__(self, other):
        return isinstance(other, Point) and (
            self.x == other.x and self.y == other.y)

    def __hash__(self):
        return hash((self.x, self.y))

    def __repr__(self):
        return f'({self.x}, {self.y})'


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
    comment += f' {layer_info.index.major}/{layer_info.index.minor}'
    return comment


def compare_x_then_y(lhs: raw_pb2.Point, rhs: raw_pb2.Point) -> bool:
    if lhs.x < rhs.x:
        return True
    return lhs.y < rhs.y


def point_on_line_at_x(start: Point, end: Point, x: int) -> Point:
    gradient = (end.y - start.y)/(end.x - start.x)
    point = Point()
    point.x = x
    point.y = int(gradient * (x - start.x) + start.y)
    return point


def swap(a: Any, b: Any) -> (Any, Any):
    return b, a


def shift_point(point: Point, diff: Point) -> Point:
    new = Point()
    new.x = point.x + diff.x
    new.y = point.y + diff.y
    return new


def shift_points(points: Sequence[Point], diff: Point) -> Sequence[Point]:
    return [shift_point(p, diff) for p in points]


def transform_points(points: Sequence[Point],
                     cut_left_x: Optional[int] = None,
                     cut_right_x: Optional[int] = None) -> Sequence[Point]:
    if not points:
        return points
    # Find the lower-left point. Make it the start.
    k = 0
    ll = points[k]
    for i, point in enumerate(points):
        if point.x <= ll.x and point.y <= ll.y:
            k = i
            ll = point

    result = points[k:] + points[:k]
    #print(f'start: {ll}\nin: {points}\nout: {ordered_points}')

    if cut_left_x is not None:
        # The last point left of the x cut-off.
        cut_last = False
        last = None
        ordered_points = []
        for i in range(len(result)):
            point = result[i]
            if point.x < cut_left_x:
                if last is not None and not cut_last:
                    x_cross = point_on_line_at_x(last, point, cut_left_x)
                    #print(f'new point {x_cross} between last non- and cut-off {point}')
                    ordered_points.append(x_cross)
                last = point
                cut_last = True
                #print(f'point{point} is < {cut_left_x} so dropped')
                continue
            if cut_last:
                x_cross = point_on_line_at_x(last, point, cut_left_x)
                #print(f'new point {x_cross} between last cut off and {point}')
                ordered_points.append(x_cross)
            ordered_points.append(point)
            last = point
            cut_last = False
        # Special consideration for the line between the last and the first point.
        first = result[0]
        if first.x < cut_left_x and not cut_last:
            x_cross = point_on_line_at_x(last, point, cut_left_x)
            #print(f'new point {x_cross} between last cut off and starting point {point}')
            ordered_points.append(x_cross)
        result = ordered_points
        if not result:
            return result

    if cut_right_x is not None:
        # Do it again for the right cut-off.
        cut_last = False
        last = None
        ordered_points = []
        for i in range(len(result)):
            point = result[i]
            if point.x > cut_right_x:
                if last is not None and not cut_last:
                    x_cross = point_on_line_at_x(last, point, cut_right_x)
                    #print(f'new point {x_cross} between last non- and cut-off {point}')
                    ordered_points.append(x_cross)
                last = point
                cut_last = True
                #print(f'point{point} is > {cut_right_x} so dropped')
                continue
            if cut_last:
                x_cross = point_on_line_at_x(last, point, cut_right_x)
                #print(f'new point {x_cross} between last cut off and {point}')
                ordered_points.append(x_cross)
            ordered_points.append(point)
            last = point
            cut_last = False
        # Special consideration for the line between the last and the first point.
        first = result[0]
        if first.x > cut_right_x and not cut_last:
            x_cross = point_on_line_at_x(last, point, cut_right_x)
            #print(f'new point {x_cross} between last cut off and starting point {point}')
            ordered_points.append(x_cross)
        result = ordered_points

    return result


def main():
    parser = argparse.ArgumentParser(description='no description')
    parser.add_argument('--layout', '-i', default='', required=True)
    parser.add_argument('--tech', '-t', default='', required=False)
    parser.add_argument('--list', '-l', default=False,
                        action=argparse.BooleanOptionalAction)
    parser.add_argument('--cell', '-c', default='', required=False)
    parser.add_argument('--offset_x', '-ox', type=int, default=0, required=False)
    parser.add_argument('--cut_left_x', type=int, default=None, required=False)
    parser.add_argument('--cut_right_x', type=int, default=None, required=False)
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
            index = layer_info.index
            layer_infos[index.major][index.minor] = layer_info

    add_polygon_start = 'layout->AddPolygon(Polygon({'
    indents = ' ' * len(add_polygon_start)

    offset = Point()
    if args.offset_x:
        offset.x = args.offset_x

    cut_left_x = args.cut_left_x
    cut_right_x = args.cut_right_x

    if args.list:
        cell_names = [c.name for c in library.cells]
        for name in sorted(cell_names):
            print(name)
        return

    for cell in library.cells:
        layout = cell.layout

        if args.cell and cell.name != args.cell:
            continue;
        
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
                ll = Point(point_pb=rect_pb.lower_left)
                ur = Point()
                ur.x = ll.x + rect_pb.width
                ur.y = ll.y + rect_pb.height

                if cut_left_x is not None:
                    if ur.x < cut_left_x:
                        continue
                    if ll.x < cut_left_x:
                        ll.x = cut_left_x
                if cut_right_x is not None:
                    if ll.x > cut_right_x:
                        continue
                    if ur.x > cut_right_x:
                        ur.x = cut_right_x

                ll = shift_point(ll, offset)
                ur = shift_point(ur, offset)

                print(f'layout->AddRectangle(Rectangle(Point({ll.x}, {ll.y}), Point({ur.x}, {ur.y})));')

            for poly_pb in shapes.polygons:
                vertices = shift_points(
                    transform_points(
                        [Point(point_pb=x) for x in poly_pb.vertices],
                        cut_left_x=cut_left_x,
                        cut_right_x=cut_right_x),
                    offset)
                if not vertices:
                    continue

                point_list = f',\n{indents}'.join(
                    f'Point({vertex.x}, {vertex.y})' for vertex in
                    vertices)
                print(f'{add_polygon_start}{point_list}}}));')

            if shapes.paths:
                # These are actually just simplified geometry::PolyLines, so we
                # could support them.
                print(f'// WARNING: ignored {len(shapes.paths)} path(s)')

            print('')

        # Annotations.


if __name__ == '__main__':
    main()
