import sys
import os
import time

#build_config="x64-Debug (default)"
build_config="x64-Release"
sys.path.append(os.path.join(os.path.dirname(__file__), "..", "out", "build", build_config, "cpp_src", "PyPovGlobe"))

import PyPovGlobe
import tile_server_api

num_leds_per_side = 55
radius = 13.25
spacing_top = 1.5
spacing_bottom = 2.0
double_sided=True

rpm = PyPovGlobe.RpmMeasureSim()
renderer = PyPovGlobe.RendererSim(rpm)
globe = PyPovGlobe.Globe(num_leds_per_side, radius, spacing_top, spacing_bottom, double_sided, renderer)

proj = PyPovGlobe.EquirectangularProjection()
interp = PyPovGlobe.NearestNeighbourPixelInterpolation()

res_dir = os.path.join(os.path.dirname(__file__), "..", "res")

all_apps = [
    PyPovGlobe.ApplicationImageRotator(os.path.join(res_dir, "img", "2_no_clouds_8k.jpg"), proj, interp),
    PyPovGlobe.ApplicationImageRotator(os.path.join(res_dir, "img", "kisspng-world-map-globe-equirectangular-projection-map.png"), proj, interp),
    PyPovGlobe.ApplicationImageViewer(os.path.join(res_dir, "img", "soccer2_sph.png"), proj, interp),
    PyPovGlobe.ApplicationImageViewer(os.path.join(res_dir, "img", "tennisenn_sph.png"), proj, interp),
    PyPovGlobe.ApplicationImageRotator(os.path.join(res_dir, "img", "1_earth_8k.jpg"), proj, interp),
    
]

globe.runRendererAsync()

app = tile_server_api.TileServerApp()
globe.runApplicationAsync(app)

while(True):
    time.sleep(1)

app_idx = 0
while(True):
    globe.stopCurrentApp()
    globe.runApplicationAsync(all_apps[app_idx])

    time.sleep(3)

    app_idx+=1
    if app_idx >= len(all_apps):
        app_idx = 0

globe.shutdown()