#
# Elasticsearch Dockerfile
#
# https://github.com/dockerfile/elasticsearch
#

# Pull base image.
#FROM daocloud.io/longsky/ubuntu:wily
#FROM dockerfile/java:oracle-java8
FROM daocloud.io/library/java:latest

ENV ES_HOME=/elasticsearch

# Install Elasticsearch.
RUN \
  useradd -d $ES_HOME -s /bin/bash -k /etc/skel -m demo && \
  mkdir -p $ES_HOME/data && chown -R demo.demo $ES_HOME

# Mount elasticsearch.yml config
ADD elasticsearch /elasticsearch
ADD elasticsearch.yml /elasticsearch/config/elasticsearch.yml

RUN chown -R demo.demo $ES_HOME

# Define mountable directories.
VOLUME $ES_HOME/data

# Define working directory.
WORKDIR $ES_HOME

# Expose ports.
#   - 9200: HTTP
#   - 9300: transport
EXPOSE 9200
EXPOSE 9300

USER demo

# Define default command.
CMD ["/elasticsearch/bin/elasticsearch", "--network.host", "0.0.0.0"]
