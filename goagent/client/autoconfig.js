// Proxy Auto-Config file 
// localnet - http://en.wikipedia.org/wiki/Private_network
// localhost - http://en.wikipedia.org/wiki/Localhost
// http://findproxyforurl.com/
function FindProxyForURL(url, host) {
	var autoproxy = ['PROXY 127.0.0.1:8087', 'PROXY 127.0.0.1:8088', 'PROXY 127.0.0.1:8089', 'PROXY 127.0.0.1:8090'];
	var blackhole = 'PROXY 127.0.0.1:8086';
	var defaultproxy = 'DIRECT';
	var whitelist_hosts = {"126.com":0.000,"126.net":0.000,"127.net":0.000,"163.com":0.000,"263.com":0.000,"263.net":0.000,"360buy.com":0.000,"360buyimg.com":0.000,"360safe.com":0.000,"58.com":0.000,"alibaba.com":0.000,"alicdn.com":0.000,"aliimg.com":0.000,"alipay.com":0.000,"alipayobjects.com":0.000,"alisoft.com":0.000,"aliyun.com":0.000,"aliyuncdn.com":0.000,"amap.com":0.000,"apache.org":0.000,"autonavi.com":0.000,"baidu.com":0.000,"baidustatic.com":0.000,"bdimg.com":0.000,"bdstatic.com":0.000,"chinamobile.com":0.000,"cnzz.com":0.000,"csc108.com":0.000,"csdn.net":0.000,"docker.com":0.000,"ease.com":0.000,"github.com":0.000,"gravatar.com":0.000,"gtimg.com":0.000,"haizhebar.com":0.000,"happyelements.com":0.000,"hexun.com":0.000,"hichina.com":0.000,"iask.com":0.000,"ifeng.com":0.000,"ifengimg.com":0.000,"images-amazon.com":0.000,"imedao.com":0.000,"ipv6-test.com":0.000,"jasonsavard.com":0.000,"jd.com":0.000,"kanimg.com":0.000,"linezing.com":0.000,"microsoft.com":0.000,"mmstat.com":0.000,"mysql.com":0.000,"netease.com":0.000,"oray.com":0.000,"php.net":0.000,"qq.com":0.000,"qzoneapp.com":0.000,"railcn.net":0.000,"sogou.com":0.000,"sogoucdn.com":0.000,"sohu.com":0.000,"sqlite.org":0.000,"ss64.com":0.000,"ssl-images-amazon.com":0.000,"suning.com":0.000,"tanx.com":0.000,"taobao.com":0.000,"taobaocdn.com":0.000,"tdimg.com":0.000,"tianyaui.com":0.000,"tieba.com":0.000,"tmall.com":0.000,"twsapp.com":0.000,"ubuntu.com":0.000,"weibo.com":0.000,"xueqiu.com":0.000,"yahoo.com":0.000,"ydstatic.com":0.000,"yimg.com":0.000,"yinhang.com":0.000,"ykimg.com":0.000,"yongche-inc.com":0.000,"yongche.com":0.000,"yongche.name":0.000,"yongche.net":0.000,"yongche.org":0.000,"youdao.com":0.000,"zhaopin.com":0.000};
	
	var blacklist_hosts = {"twitter.com":1,"wrating.com":1,"biddingx.com":1};
	var google_domains = {
		"google.com":1,
		"googleapis.com":1,
		"googlecode.com":1,
		"googlegroups.com":1,
		"appspot.com":1,
		"gstatic.com":1,
		"google-analytics.com":1,
		"googleusercontent.com":1,
		"googletagservices.com":1,
		"googletagmanager.com":1,
		"gmail.com":1,
		"android.com":1
	};
	if (IsHostInDomains(host, blacklist_hosts)) {
		return blackhole;
	}
	else if (IsHostInDomains(host,whitelist_hosts) || shExpMatch(host, '*.cn') || isPlainHostName(host) ||
        host.indexOf('127.') == 0 || host.indexOf('192.168.') == 0 || host.indexOf('10.') == 0 ||
        isInNet(host, "172.16", "255.240.0.0") || shExpMatch(host, "[0-9]*.[0-9]*.[0-9]*.[0-9]*") ) 
    {
        return defaultproxy;
    }    
    else 
    {
    	if (IsHostInDomains(host, google_domains) || shExpMatch(host, '*google*')) {
    		return autoproxy[0];
    	}
    	else if (shExpMatch(host, '*.com')) {
    		return autoproxy[1];
    	}
    	else if (shExpMatch(host, '*.net') || shExpMatch(host, '*.org')) {
    		return autoproxy[2];
    	}
    	else {
    		return autoproxy[3];
    	}
    }
	
	function IsHostInDomains(host, domains) {
		var pos;
		do {
			if (domains.hasOwnProperty(host)) {
				return true;
			}
			pos = host.indexOf('.') + 1;
			host = host.slice(pos);
		} while (pos >= 1);
		return false;
	}
}
