export PROJECT_ROOT=/home/thomasokeeffe/workspace/roguelike
export PROJECT_BUILD_DIR=$PROJECT_ROOT/build
export PROJECT_SOURCE_DIR=$PROJECT_ROOT/src
export CLANGD_FLAGS="--compile-commands-dir=$PROJECT_BUILD_DIR"
export MAKEPRG="cd \$PROJECT_ROOT; make"
