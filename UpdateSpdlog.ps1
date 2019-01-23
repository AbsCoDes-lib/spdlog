
# Download files from https://github.com/gabime/spdlog
$Version="v1.3.1"
docker run -it --rm -e http_proxy=${Env:\HTTP_PROXY} -e https_proxy=${Env:\HTTPS_PROXY} -v $PSScriptRoot/include/:/data craftdock/alpine-data gh-downloader -u gabime -r spdlog -b $Version -p include -o ./

# Remove bundle
Remove-Item -Path $PSScriptRoot\include\spdlog\fmt\bundled -Recurse -Force