{ pkgs ? import <nixpkgs> { } }:

pkgs.mkShell {
  nativeBuildInputs = [
    pkgs.bear
    pkgs.libgcc
    pkgs.gnumake
    pkgs.python3
  ];
}
