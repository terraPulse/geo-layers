FROM 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l0

ADD bin /opt/bin
ENV PATH=$PATH:/opt/bin

RUN apt-get update -y  && apt-get install --no-install-recommends -y libshp2
