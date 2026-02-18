from hdl21.prefix import m, u, n, p, f
import hdl21 as h

def scale_params(module: h.Module):
    SCALE = 1E6
    for name, instance in module.instances.items():
        for param, value in instance.of.params.items():
            new_value = (value * SCALE).scale(m)
            instance.of.params[param] = new_value
