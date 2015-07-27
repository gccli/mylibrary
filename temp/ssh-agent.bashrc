if [ -f $SSH_ENV ]; then
    source $SSH_ENV
    ps -ef | grep $SSH_AGENT_PID | grep ssh-agent$ > /dev/null || {
        start_agent
	check_keys
    }
else
    start_agent
    check_keys
fi
