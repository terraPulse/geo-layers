FROM pswang/geo-layers:l1

WORKDIR /opt
RUN mkdir /opt/tmp

ADD bin /opt/bin
ENV PATH=$PATH:/opt/bin
ENV TMP=/opt/tmp

RUN apt-get update -y  && apt-get install --no-install-recommends -y libshp2
