# prepare ansible master environment

Install anisble.

<pre>apt -y install ansible</pre>

Create priv keys for windows and WSL guest

<pre>ssh-keygen</pre>

Install keys on remote server

<pre>ssh-copyid 10.1.1.207</pre>

ensure user can sudo without password (or use alternate command to provide credentials). Im not 100% if this is needed, check later.


# run yaml playbook on slave (without .ini file)

ansible-playbook -i '10.1.1.203,' fs.yaml
