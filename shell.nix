{ stdenv, pkgs, lib }:

pkgs.mkShell rec {
  nativeBuildInputs = with pkgs; [
    gcc
    clang-tools
    cmake
    gnumake
    icu
    libcxx
    gdb
  ];
  CPATH = lib.makeSearchPathOutput "dev" "include" nativeBuildInputs;
  LD_LIBRARY_PATH = lib.makeLibraryPath [stdenv.cc.cc.lib];
}
