#
# Elasticsearch Dockerfile
#
# https://github.com/dockerfile/elasticsearch
#

# Pull base image.
#FROM daocloud.io/longsky/ubuntu:wily
#FROM dockerfile/java:oracle-java8
FROM daocloud.io/library/java:latest

ENV ES_VERSION=2.1.0

# Install Elasticsearch.
RUN \
  cd /tmp && mkdir -p /data && \
  wget https://download.elasticsearch.org/elasticsearch/elasticsearch/elasticsearch-$ES_VERSION.tar.gz && \
  tar xvzf elasticsearch-$ES_VERSION.tar.gz && \
  rm -f elasticsearch-$ES_VERSION.tar.gz && \
  mv /tmp/elasticsearch-$ES_VERSION /elasticsearch && \
  useradd -s /bin/bash -k /etc/skel -m demo && \
  chown -R demo.demo /data && \
  chown -R demo.demo /elasticsearch


# Define mountable directories.
VOLUME ["/data"]

# Mount elasticsearch.yml config
ADD config/elasticsearch.yml /elasticsearch/config/elasticsearch.yml
COPY run.sh /

# Define working directory.
WORKDIR /data

# Define default command.
CMD ["/run.sh"]

# Expose ports.
#   - 9200: HTTP
#   - 9300: transport
EXPOSE 9200
EXPOSE 9300
