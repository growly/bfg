import cairosvg
import imageio.v3 as iio
import gdspy
import subprocess
import os

SKY130_SVG_STYLE = {
    # areaid.standardc
    (81, 4): {
        'fill': None,
        'stroke': 'red'
    },
}


def make_svg(gds_path: str, image_path: str):
    library = gdspy.GdsLibrary(infile=gds_path)
    cell = library.cells['Sky130TransmissionGate']
    cell.write_svg(
        image_path,
        scaling=500,
        background=None,
        style=SKY130_SVG_STYLE
    )


def make_png(svg_path: str, png_path: str):
    cairosvg.svg2png(url=svg_path, write_to=png_path)


def make_subdirs():
    for subdir in ('gds', 'pb.txt', 'svg', 'png', 'gif'):
        try:
            os.mkdir(subdir)
        except FileExistsError:
            pass


def make_gif(png_paths: str, gif_name):
    images = [iio.imread(path) for path in png_paths]
    iio.imwrite(gif_name, images, duration=500, loop=0)


def sweep():
    make_subdirs()

    svgs = []
    pngs = []

    i = 0
    for stacks in ('true', 'false'):
        for add in range(0, 301, 50):
            params = f'''\
p_width_nm: {700 + add};
p_length_nm: 150;
n_width_nm: {350 + add};
n_length_nm: 150;

stacks_left: {stacks};
stacks_right: {stacks};

vertical_tab_pitch_nm: 400;
vertical_tab_offset_nm: 200;
poly_pitch_nm: 300;

draw_nwell: false;

p_tab_position: NORTH;
n_tab_position: SOUTH;
'''
            param_path = f'pb.txt/{i}.pb.txt'
            with open(param_path, 'w') as f:
                f.write(params)

            image_path = f'svg/{i}.svg'

            out_prefix = 'transmission_gate'

            subprocess.run(
                [
                    '../../build/bfg',
                    '--technology', '../../sky130.technology.pb',
                    '--primitives', '../../sky130.primitives.pb',
                    '--external_circuits', '../../sky130hd.pb',
                    '--write_text_format',
                    '--run_generator', 'Sky130TransmissionGate',
                    '--params', param_path,
                    '--output_library', out_prefix
                ]
            );

            gds_path = f'gds/{i}.gds'

            subprocess.run(
                [
                    '/home/arya/src/Layout21/target/debug/proto2gds',
                    '--verbose',
                    '-i', f'{out_prefix}.library.pb',
                    '-t', '../../sky130.technology.pb',
                    '-o', gds_path
                ]
            )

            make_svg(gds_path, image_path)
            svgs.append(image_path)
            i = i + 1


    for svg_path in svgs:
        png_path = svg_path.replace('.svg', '.png').replace('svg/', 'png/')
        make_png(svg_path, png_path)
        pngs.append(png_path)

    #make_gif(pngs, 'all.gif')


if __name__ == '__main__':
    sweep()
