param (
    [string]$project = ""
 )

$env:DESTDIR = "$env:MESON_BUILD_ROOT\.package"

Push-Location $env:MESON_BUILD_ROOT
meson install

Move-Item -Path .package -Destination $project -Force

Compress-Archive -DestinationPath .\$($project).zip -Path $($project) -Force
Remove-Item -Force -Recurse $($project)

Pop-Location
