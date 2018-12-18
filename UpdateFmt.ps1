
# Download files from https://github.com/gabime/spdlog
docker run -it --rm -e http_proxy=${Env:\HTTP_PROXY} -e https_proxy=${Env:\HTTPS_PROXY} -v $PSScriptRoot/include/:/data craftdock/alpine-data gh-downloader -u gabime -r spdlog -p include -o ./
