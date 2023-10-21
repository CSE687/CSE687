# .devcontainer

## Getting Started

1. `.devcontainer` must be a top level directory in a VSCode workspace.

    ```bash
    tree -a -L 1 .
    .
    ├── .devcontainer # Top level directory in VSCode's File Explorer (Cmd+Shift+E)
    ├── projectA
    └── projectB
    ```

    The running container will have access to `.devcontainer`, `projectA` & `projectB` via a bind mount.
2. Install VSCode extension `Dev Containers`.
2. Reopen in Container using the VSCode Command Pallete (`Cmd+Shift+P` or `Ctrl+Shift+P`):
    - `Dev Containers: Reopen in Container`

## Common Issues

### Missing CLI Utilities

- **Solution**: Close and reopen a new terminal.

The terminal likely started using `bash` or `sh` rather than `zsh`. This is a known bug in VSCode `.devcontainer`.

- Links
    - [https://github.com/microsoft/vscode-remote-release/issues/6882](https://github.com/microsoft/vscode-remote-release/issues/6882)

***

### Docker Isn't Running

```
Cannot connect to the Docker daemon at unix:///Users/.../.docker/run/docker.sock. Is the docker daemon running?
```

- **Solution**: Verify Docker is running on host machine.

***

### Docker Container Does Not Start

Error mentions `docker-image://docker.io/docker/dockerfile:1.4`

- **Solution**: Pre-downloading dockerfile will likely resolve.

```bash
docker pull docker/dockerfile:1.4
```

***

### Github Certificate Issues

Error

```bash
fatal: unable to access 'https://github.com/xxx/xxx.git': server certificate verification failed. CAfile: none CRLfile: none
```

- **Solution**: Restart host machine.

If unresolved after restart:

The error from the git client will be resolved if you add the certs from the remote git server to the list of locally checked certificates. This is done by using `openssl` to pull the certificates from the remote host. This requires a change to `.devcontainer/Dockerfile`.

```bash
# Pull cert from Github
openssl s_client -showcerts -servername git.mycompany.com -connect git.mycompany.com:443 </dev/null 2>/dev/null | sed -n -e '/BEGIN\ CERTIFICATE/,/END\ CERTIFICATE/ p'  > git-mycompany-com.pem
# Add to bundle
cat github-com.pem | sudo tee -a /etc/ssl/certs/ca-certificates.crt
```

- Links
    - [https://fabianlee.org/2019/01/28/git-client-error-server-certificate-verification-failed/](https://fabianlee.org/2019/01/28/git-client-error-server-certificate-verification-failed/)

***

### Dockerfile-with-features

Mentions:

- `Dockerfile-with-features`
- `ERROR: failed to solve: rpc error: code = Unknown desc = failed to solve with frontend dockerfile.v0: failed to create LLB definition: target stage dev_containers_target_stage could not be found`

Resolution:

- Remove any rogue `\` in the `Dockerfile`.
- Check `RUN` statments for trailing `\` with no next line.
- Add missing `\` to `Dockerfile`.

```bash
# Example
RUN apt-get install
    package1 \
    package2 \  # Issue is here

RUN apt-get install
    package1    # Issue is here
    package2 \
    package3
```
