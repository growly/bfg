import cairosvg
import imageio.v3 as iio
import gdspy
import subprocess
import os
import sys
import itertools
from PIL import Image, ImageFont, ImageDraw

SKY130_SVG_STYLE = {
    # areaid.standardc
    (81, 4): {
        'fill': None,
        'stroke': 'red'
    },
    # npc.drawing
    (95, 20): {
        'fill': None,
        'stroke': 'orange'
    }
}

PNG_FONT = ImageFont.truetype("TerminusTTF-4.46.0.ttf", 16)


def gds2svg(cell_name: str, gds_path: str, image_path: str):
    library = gdspy.GdsLibrary(infile=gds_path)
    cell = library.cells[cell_name]
    cell.write_svg(
        image_path,
        scaling=500,
        background=None,
        style=SKY130_SVG_STYLE
    )


def make_png(svg_path: str, png_path: str):
    try:
        cairosvg.svg2png(url=svg_path, write_to=png_path)
    except:
        print(f'error: could not make png out of {svg_path}')
        sys.exit(1)


def make_subdirs():
    for subdir in ('gds', 'pb.txt', 'svg', 'png', 'gif', 'frames'):
        try:
            os.mkdir(subdir)
        except FileExistsError:
            pass


def make_gif(png_paths: str, gif_name, all_params,
             centred_crop: bool = False):
    sources = [Image.open(png) for png in png_paths]

    max_width = max(image.size[0] for image in sources)
    max_height = max(image.size[1] for image in sources)

    print(f'max width and height are {max_width} x {max_height}')

    images = []

    white_background = Image.new(
        'RGBA', (max_width, max_height), (255, 255, 255))

    get_params = (
        lambda i: all_params[i]
        if all_params and i < len(all_params) else None)

    paths = []
    for i, source in enumerate(sources):
        # (0, 0) is top left, y grows down
        if centred_crop:
            width, length = source.size
            wing = (max_width - width) / 2
            cropped = source.crop((-wing, 0, max_width - wing, max_height))
        else:
            cropped = source.crop((0, 0, max_width, max_height))

        image = Image.alpha_composite(white_background, cropped)
        image = image.convert("P", palette=Image.WEB)

        params = get_params(i)
        if params:
            draw = ImageDraw.Draw(image)
            draw.text((0, 0), params, (0, 0, 0), font=PNG_FONT)
            
        images.append(image)

        frame_path = source.filename.replace('png/', 'frames/') 
        image.save(frame_path)
        paths.append(frame_path)
        print(f'processed frame {i}: {frame_path}')

    #images[0].save('animation.gif',
    #               save_all=True,
    #               append_images=images[1:],
    #               optimize=False,
    #               duration=1000,
    #               #disposal=2,
    #               #palette=ImagePalette.ImagePalette('RGB', (
    #               transparency=0,
    #               loop=0)

    images = [iio.imread(path) for path in paths]
    iio.imwrite(gif_name, images, duration=200, loop=0)


def make_for_params(generator_name: str, name: str, params: str, svgs, pngs):
    param_path = f'pb.txt/{name}.pb.txt'
    with open(param_path, 'w') as f:
        f.write(params)

    image_path = f'svg/{name}.svg'

    out_prefix = 'transmission_gate'

    bfg_stdout = ''
    bfg_result = subprocess.run(
        [
            '../../build/bfg',
            '--technology', '../../sky130.technology.pb',
            '--primitives', '../../sky130.primitives.pb',
            '--external_circuits', '../../sky130hd.pb',
            '--write_text_format',
            '--run_generator', generator_name,
            '--params', param_path,
            '--output_library', out_prefix,
            '--logtostderr'
        ],
        encoding='UTF-8',
        capture_output=True
    );

    
    if bfg_result.returncode != 0:
        print(param_path)
        print(bfg_stdout)
        sys.exit(1)

    gds_path = f'gds/{name}.gds'

    subprocess.run(
        [
            '/home/arya/src/Layout21/target/debug/proto2gds',
            '--verbose',
            '-i', f'{out_prefix}.library.pb',
            '-t', '../../sky130.technology.pb',
            '-o', gds_path
        ]
    )

    gds2svg(generator_name, gds_path, image_path)
    svgs.append(image_path)



def sweep_transmission_gate():
    make_subdirs()

    svgs = []
    pngs = []
    all_params = []

    tab_positions = ('NORTH', 'SOUTH')

    i = 0
    for stacks_left in ('true', 'false'):
        for stacks_right in ('true', 'false'):
            for p_tab, n_tab in itertools.product(tab_positions, tab_positions):
                for pitch in (300, 340, 400):
                    for add in range(0, 301, 50):
                        params = f'''\
p_width_nm: {700 + add};
p_length_nm: 150;
n_width_nm: {350 + add};
n_length_nm: 150;

stacks_left: {stacks_left};
stacks_right: {stacks_right};

vertical_tab_pitch_nm: {pitch};
vertical_tab_offset_nm: 200;
poly_pitch_nm: 300;

draw_nwell: false;

p_tab_position: {p_tab};
n_tab_position: {n_tab};
'''
                        make_for_params('Sky130TransmissionGate',
                                        f'{i}', params, svgs, pngs)
                        all_params.append(params)
                        i = i + 1

    for svg_path in svgs:
        png_path = svg_path.replace('.svg', '.png').replace('svg/', 'png/')
        make_png(svg_path, png_path)
        pngs.append(png_path)

    make_gif(pngs, 'transmission_gate.gif', all_params, centred_crop=True)


def sweep_transmission_gate_stack():
    make_subdirs()

    svgs = []
    pngs = []
    all_params = []

    i = 0
    net_sequence = ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L",
                    "M", "N", "O"]
    for vertical_pitch in (None, 340, 400):
        for j in range(2, int(len(net_sequence) / 2)):
            sequence = net_sequence[0:2*j + 1]
            for add in range(0, 351, 50):
                p_width_nm = 650 + add
                n_width_nm = 350 + add
                params = '\n'.join(f'net_sequence: "{net}"'
                                   for net in sequence)
                params += f'''
p_width_nm: {p_width_nm}
p_length_nm: 150
n_width_nm: {n_width_nm}
n_length_nm: 150
'''
                if vertical_pitch:
                    params += f'''
vertical_pitch_nm: {vertical_pitch}
horizontal_pitch_nm: {vertical_pitch}
'''
                make_for_params('Sky130TransmissionGateStack',
                                f'{i}', params, svgs, pngs)
                all_params.append(params)
                i = i + 1

    for svg_path in svgs:
        png_path = svg_path.replace('.svg', '.png').replace('svg/', 'png/')
        make_png(svg_path, png_path)
        pngs.append(png_path)

    make_gif(
        pngs, 'transmission_gate_stack.gif', all_params, centred_crop=False)


if __name__ == '__main__':
    sweep_transmission_gate()
    sweep_transmission_gate_stack()
