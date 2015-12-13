## Push images to daocloud.io ##

longsky is my username in daocloud.io

> docker tag <your-image> daocloud.io/longsky/<your-image>:<tag>
> docker push daocloud.io/longsky/<your-image>:<tag>

docker-tag - Tag an image into a repository

docker tag [-f|--force[=false]] [--help] IMAGE[:TAG] [REGISTRY_HOST/][USERNAME/]NAME[:TAG]

REGISTRY_HOST
          The hostname of the registry if required. This may also include the port separated by a ':'

USERNAME
          The username or other qualifying identifier for the image.

NAME
          The image name.
