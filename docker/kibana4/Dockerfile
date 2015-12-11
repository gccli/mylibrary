#
# Kibana4 Dockerfile
#
# http://www.elasticsearch.org/overview/kibana/installation/
#

# Pull base image.
FROM daocloud.io/library/java:latest

ENV KIBANA_VERSION=4.3.0-linux-x64

#
# Download kibana4 in current directory
# https://download.elastic.co/kibana/kibana/kibana-4.3.0-linux-x64.tar.gz
#

ADD kibana-$KIBANA_VERSION.tar.gz /

# Install Kibana4
RUN mv /kibana-$KIBANA_VERSION /kibana

# Define default command.
CMD ["/kibana/bin/kibana"]

# Expose ports.
EXPOSE 5601
