import gdspy
import subprocess

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

def sweep():
    i = 0
    for p_width_nm in range(400, 1001, 50):
        for n_width_nm in range(350, 651, 50):
            params = f'''\
p_width_nm: {p_width_nm};
p_length_nm: 150;
n_width_nm: {n_width_nm};
n_length_nm: 150;

stacks_left: false;
stacks_right: false;

vertical_tab_pitch_nm: 400;
vertical_tab_offset_nm: 200;
poly_pitch_nm: 300;

draw_nwell: false;

p_tab_position: NORTH;
n_tab_position: SOUTH;
'''
            param_path = f'{i}.pb.txt'
            with open(param_path, 'w') as f:
                f.write(params)

            image_path = f'{i}.svg'

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

            gds_path = f'{i}.gds'

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
            i = i + 1

if __name__ == '__main__':
    sweep()
