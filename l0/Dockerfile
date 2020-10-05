FROM ubuntu:focal

ENV DEBIAN_FRONTEND noninteractive
ENV TZ America/New_York

RUN apt-get update -y && apt-get install -y \
    python3-gdal \
    python3-boto3 \
    libgdal-dev
