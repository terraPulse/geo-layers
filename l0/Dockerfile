FROM public.ecr.aws/ubuntu/ubuntu:focal

ENV DEBIAN_FRONTEND noninteractive
ENV TZ America/New_York

WORKDIR /opt
RUN mkdir /opt/tmp

ENV TMP=/opt/tmp

RUN apt-get update -y && apt-get install -y --no-install-recommends python-is-python3 python3-gdal python3-boto3 awscli gdal-bin
