#
# Kibana4 Dockerfile
#
# http://www.elasticsearch.org/overview/kibana/installation/
#

# Pull base image.
FROM dockerfile/java:oracle-java7

# Install Kibana4
RUN \
  cd /tmp && \
  wget https://download.elasticsearch.org/kibana/kibana/kibana-4.0.0-BETA1.1.tar.gz && \
  tar xvzf kibana-4.0.0-BETA1.1.tar.gz && \
  rm -f kibana-4.0.0-BETA1.1.tar.gz && \
  mv /tmp/kibana-4.0.0-BETA1.1 /kibana && \
  sed -i 's/^kibanaIndex.*/kibanaIndex: "kibana4"/' /kibana/config/kibana.yml

# Define mountable directories.
VOLUME ["/data"]

# Define working directory.
WORKDIR /data

# Define default command.
CMD ["/kibana/bin/kibana"]

# Expose ports.
EXPOSE 5601
