import cairosvg
import imageio.v3 as iio
import gdspy
import subprocess
import os
import PIL

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
    for subdir in ('gds', 'pb.txt', 'svg', 'png', 'gif', 'frames'):
        try:
            os.mkdir(subdir)
        except FileExistsError:
            pass


def make_gif(png_paths: str, gif_name):
    sources = [PIL.Image.open(png) for png in png_paths]

    max_width = max(image.size[0] for image in sources)
    max_height = max(image.size[1] for image in sources)

    print(f'max width and height are {max_width} x {max_height}')

    images = []

    white_background = PIL.Image.new(
        'RGBA', (max_width, max_height), (255, 255, 255))

    paths = []
    for source in sources:
        # (0, 0) is top left, grows down
        cropped = source.crop((0, 0, max_width, max_height))
        image = PIL.Image.alpha_composite(white_background, cropped)
        image = image.convert("P", palette=PIL.Image.WEB)
        images.append(image)

        frame_path = source.filename.replace('png/', 'frames/') 
        image.save(frame_path)
        paths.append(frame_path)
        print(frame_path)

    #images[0].save('animation.gif',
    #               save_all=True,
    #               append_images=images[1:],
    #               optimize=False,
    #               duration=1000,
    #               #disposal=2,
    #               #palette=PIL.ImagePalette.ImagePalette('RGB', (
    #               transparency=0,
    #               loop=0)

    images = [iio.imread(path) for path in paths]
    iio.imwrite(gif_name, images, duration=500, loop=0)


def make_for_params(name: str, params: str, svgs, pngs):
    param_path = f'pb.txt/{name}.pb.txt'
    with open(param_path, 'w') as f:
        f.write(params)

    image_path = f'svg/{name}.svg'

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

    make_svg(gds_path, image_path)
    svgs.append(image_path)



def sweep():
    make_subdirs()

    svgs = []
    pngs = []

    i = 0
    for stacks_left in ('true', 'false'):
        for stacks_right in ('true', 'false'):
            for tab_position in ('NORTH', 'SOUTH'):
                for add in range(0, 301, 50):
                    for pitch in range(100, 500, 30):
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

p_tab_position: {tab_position};
n_tab_position: {tab_position};
'''
                        make_for_params(f'{i}', params, svgs, pngs)
                        i = i + 1

    for svg_path in svgs:
        png_path = svg_path.replace('.svg', '.png').replace('svg/', 'png/')
        make_png(svg_path, png_path)
        pngs.append(png_path)

    make_gif(pngs, 'all.gif')


if __name__ == '__main__':
    sweep()
