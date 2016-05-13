* Thu May 12 13:42:45 CST 2016

Install Elasticsearch and Kibana with repositories
==================================================
[setup-repositories][1]

1. Download and install the Public Signing Key

        wget -qO - https://packages.elastic.co/GPG-KEY-elasticsearch | sudo apt-key add -
        apt-key list


2. Save the repository definition to /etc/apt/sources.list.d/elasticsearch-2.x.list

        echo "deb https://packages.elastic.co/elasticsearch/2.x/debian stable main" | sudo tee -a /etc/apt/sources.list.d/elasticsearch.list
        echo "deb http://packages.elastic.co/kibana/4.5/debian stable main" | sudo tee -a /etc/apt/sources.list.d/elasticsearch.list


3. Run apt-get update and the repository is ready for use. You can install it with:

        apt-get update && sudo apt-get install elasticsearch kibana

4. Configure Elasticsearch to automatically start during bootup.

        sudo /bin/systemctl daemon-reload
        sudo /bin/systemctl enable elasticsearch.service
        sudo /bin/systemctl enable kibana.service

[1]: https://www.elastic.co/guide/en/elasticsearch/reference/current/setup-repositories.html#setup-repositories "Setup Repositories"


Install with tar
================
