from hdl21.prefix import m, u, n, p, f
import hdl21 as h

def scale_params(module: h.Module):
    SCALE = 1E6

    if 'params' in module.__dict__:
        for param, value in module.params.items():
            new_value = (value * SCALE).scale(m)
            module.params[param] = new_value

    if 'instances' in module.__dict__:
        for name, instance in module.instances.items():
            scale_params(instance.of)
