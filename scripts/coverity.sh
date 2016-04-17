#!/bin/bash

set -e

PLATFORM=`uname`
TOOL_ARCHIVE=/tmp/cov-analysis-${PLATFORM}.tgz
TOOL_URL=https://scan.coverity.com/download/${PLATFORM}
TOOL_BASE=/tmp/coverity-scan-analysis
UPLOAD_URL="https://scan.coverity.com/builds"
SCAN_URL="https://scan.coverity.com"
RESULTS_DIR="cov-int"
PATH="${TOOL_BASE}/bin/:${PATH}"

MODE=
while [ -n "$1" ]; do
  case "$1" in
    -d | --download )
        MODE=download; shift ;;
    -u | --upload )
        MODE=upload; shift ;;
    --token )
        COVERITY_SCAN_TOKEN="$2"; shift 2 ;;
    --name )
        COVERITY_SCAN_PROJECT_NAME="$2"; shift 2 ;;
    -b | --build-command )
        MODE=build ; shift ;;
    * )
        echo "Unknown argument \"$1\""; exit 1 ;;
  esac
done

# Environment check
[ -z "$COVERITY_SCAN_PROJECT_NAME" ] && echo "ERROR: COVERITY_SCAN_PROJECT_NAME must be set" && exit 1
[ -z "$COVERITY_SCAN_TOKEN" ] && echo "ERROR: COVERITY_SCAN_TOKEN must be set" && exit 1
[ -z "$COVERITY_SCAN_NOTIFICATION_EMAIL" ] && echo "ERROR: COVERITY_SCAN_NOTIFICATION_EMAIL must be set" && exit 1

# Verify upload is permitted
function cov_check_permitted {
  AUTH_RES=`curl -s --form project="$COVERITY_SCAN_PROJECT_NAME" --form token="$COVERITY_SCAN_TOKEN" $SCAN_URL/api/upload_permitted`
  if [ "$AUTH_RES" = "Access denied" ]; then
    echo -e "\033[33;1mCoverity Scan API access denied. Check COVERITY_SCAN_PROJECT_NAME and COVERITY_SCAN_TOKEN.\033[0m"
    exit 1
  else
    AUTH=`echo $AUTH_RES | ruby -e "require 'rubygems'; require 'json'; puts JSON[STDIN.read]['upload_permitted']"`
    if [ "$AUTH" = "true" ]; then
      echo -e "\033[33;1mCoverity Scan analysis authorized per quota.\033[0m"
    else
      WHEN=`echo $AUTH_RES | ruby -e "require 'rubygems'; require 'json'; puts JSON[STDIN.read]['next_upload_permitted_at']"`
      echo -e "\033[33;1mCoverity Scan analysis NOT authorized until $WHEN.\033[0m"
      exit 0
    fi
  fi
}

# Download Coverity Scan Analysis Tool
function cov_download {
  mkdir -p $TOOL_BASE
  pushd $TOOL_BASE

  echo -e "\033[33;1mDownloading Coverity Scan Analysis Tool...\033[0m"
  wget -nv -O $TOOL_ARCHIVE $TOOL_URL --post-data "project=$COVERITY_SCAN_PROJECT_NAME&token=$COVERITY_SCAN_TOKEN"

  # Extract Coverity Scan Analysis Tool
  echo -e "\033[33;1mExtracting Coverity Scan Analysis Tool...\033[0m"

  tar -xz --strip 1 -f $TOOL_ARCHIVE
  popd
}

# Upload result dir
function cov_upload {
    cov-import-scm --dir $1 --scm git --log $RESULTS_DIR/scm_log.txt 2>&1

    SHA=`git rev-parse --short HEAD`

    # Upload results
    echo -e "\033[33;1mUploading Coverity Scan Analysis results...\033[0m"
    response=$(tar cz $RESULTS_DIR | curl \
      --silent --write-out "\n%{http_code}\n" \
      --form project=$COVERITY_SCAN_PROJECT_NAME \
      --form token=$COVERITY_SCAN_TOKEN \
      --form email=$COVERITY_SCAN_NOTIFICATION_EMAIL \
      --form file=@- \
      --form version=$SHA \
      --form description="Travis CI build" \
      $UPLOAD_URL)
    status_code=$(echo "$response" | sed -n '$p')
    if [ "$status_code" != "201" ]; then
      TEXT=$(echo "$response" | sed '$d')
      echo -e "\033[33;1mCoverity Scan upload failed: $TEXT.\033[0m"
      exit 1
    fi
}

case "$MODE" in
  download )
    cov_download ;;
  upload )
    cov_upload $RESULTS_DIR ;;
  build )
    # Check we're permitted to upload, return a build command if we are
    if cov_check_permitted >& 2; then
        echo "${TOOL_BASE}/bin/cov-build --dir ${RESULTS_DIR} ${COV_BUILD_OPTIONS}"
    fi ;;
esac
