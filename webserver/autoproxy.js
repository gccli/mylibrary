function FindProxyForURL(url, host) {
    var nginx = "PROXY 10.16.13.18:8080";
    var squid = "PROXY 192.168.1.102:3128";

    var patterns = {
      'google': nginx,
      'github\.com$': squid
    };

    for (var regex in patterns) {
        if (host.match(regex)) {
            logstr = host + " match regex /" + regex + "/ using " + patterns[regex];
            alert(logstr);
            return patterns[regex];
        }
    }

    return 'DIRECT'
}
