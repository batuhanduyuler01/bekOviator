#!/bin/bash
install_pistache_from_git() {
  echo "Starting to install pistache"
  if [ $1 -eq 0 ]; then
    read -p "Do you want to install Pistache ? <y/N> " prompt
    OPTION="-y"
  else
    prompt='y'
    OPTION="-y"
  fi
  if [ $2 -eq 0 ]; then
    debug=0
  else
    debug=1
  fi

  if [[ $prompt =~ [yY](es)* ]]; then
    GIT_URL=https://github.com/oktal/pistache.git
    echo "Install Pistache from $GIT_URL"
    pushd /tmp
    echo "Downloading Pistache"
    if [[ $OPTION =~ -[yY](es)* ]]; then
      rm -rf pistache
    fi

    git clone $GIT_URL
    cd pistache && git checkout e18ed9baeb2145af6f9ea41246cf48054ffd9907
    ret=$?;[[ $ret -ne 0 ]] && popd && return $ret
    mkdir _build && cd _build
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release \
        -DPISTACHE_BUILD_EXAMPLES=false \
        -DPISTACHE_BUILD_TESTS=false \
        -DPISTACHE_BUILD_DOCS=false \
        ..
    ret=$?;[[ $ret -ne 0 ]] && popd && return $ret
    make -j $(nproc)
    ret=$?;[[ $ret -ne 0 ]] && popd && return $ret
    make install
    ret=$?;[[ $ret -ne 0 ]] && popd && return $ret
    popd
    rm -Rf /tmp/pistache/_build
  fi
  echo "pistache installation complete"
  return 0
}

install_pistache_from_git 1 1
