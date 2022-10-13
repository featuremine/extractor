if is_windows
    pkg_gen = files('pkg_gen.ps1')
    pkg_cmd = ['powershell',pkg_gen, '-project', meson.project_name()]
else
    doc = files('doc.sh')
    pkg_gen = files('pkg_gen.sh')
    pkg_cmd = [pkg_gen, meson.project_name()]
    style = files('style.sh')
    wheel_gen = files('wheel_gen.py')
    conda_pkg_gen = files('conda_pkg_gen.sh')
    build_wheel = files('build_wheel.sh')
endif

