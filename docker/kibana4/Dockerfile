#
# Kibana4 Dockerfile
#
# http://www.elasticsearch.org/overview/kibana/installation/
#

# Pull base image.
FROM daocloud.io/library/java:latest

ENV KIBANA_VERSION=4.3.0-linux-x64

# https://download.elastic.co/kibana/kibana/kibana-4.3.0-linux-x64.tar.gz

# Install Kibana4
RUN \
  cd /tmp && \
  wget https://download.elasticsearch.org/kibana/kibana/kibana-$KIBANA_VERSION.tar.gz && \
  tar xvzf kibana-$KIBANA_VERSION.tar.gz && \
  rm -f kibana-$KIBANA_VERSION.tar.gz && \
  mv /tmp/kibana-$KIBANA_VERSION /kibana

# Define mountable directories.
VOLUME ["/data"]

# Define working directory.
WORKDIR /data

# Define default command.
CMD ["/kibana/bin/kibana"]

# Expose ports.
EXPOSE 5601
