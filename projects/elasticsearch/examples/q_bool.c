{
    "query": {
        "filtered": {
            "filter":   {
                "bool": {
                    "must":     { "term":  { "folder": "inbox" }},
			"must_not": {
			    "query": { 
				"match": { "email": "urgent business proposal" }
			    }
			}
                }
            }
        }
    }
}

