#!/usr/bin/env bash

PATH=$PATH:/usr/local/bin

# hardcoded options
REPO_URL="https://github.com/blackgeorge-boom/llvm-project.git"
CHECKOUT_DIR="llvm-project-docker"
BRANCH="development"

# user-defined/detectable options
HOME_DIR=${HOME}

usage() {
  echo "Usage: $0 [-d home_dir]" 1>&2
  exit 1
}

while getopts ":d:" o; do
  case "${o}" in
  d)
    HOME_DIR=${OPTARG}
    [[ ! -d ${HOME_DIR} ]] && echo "${HOME_DIR} is not a directory!" && usage
    ;;
  *)
    usage
    ;;
  esac
done
shift $((OPTIND - 1))

CERT_FILE=${HOME_DIR}/.ssh/id_ed25519_github_docker

[[ ! -f ${CERT_FILE} ]] && echo "${CERT_FILE} does not exist!" && exit 2

eval $(keychain --eval)

pushd /tmp

if [[ ! -d ${CHECKOUT_DIR} ]]; then
  git clone ${REPO_URL} --branch ${BRANCH} ${CHECKOUT_DIR}
else
  git -C ${CHECKOUT_DIR} checkout ${BRANCH}
  git -C ${CHECKOUT_DIR} pull
fi

pushd ${CHECKOUT_DIR}/llvm/utils/docker/unasl/compiler

ssh-add ${CERT_FILE}

docker buildx build . \
  --no-cache \
  --platform linux/arm64 \
  --ssh default=${SSH_AUTH_SOCK} \
  --tag compor/unasl-compiler:latest \
  --push
