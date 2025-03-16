{
  description = "Symmetry detection and breaking library and binary for CNF formulas";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };
  outputs =
    {
      self,
      nixpkgs,
    }:
    let
      inherit (nixpkgs) lib;
      systems = lib.intersectLists lib.systems.flakeExposed lib.platforms.linux;
      forAllSystems = lib.genAttrs systems;
      nixpkgsFor = forAllSystems (system: nixpkgs.legacyPackages.${system});
      fs = lib.fileset;
      breakid-package =
        {
          stdenv,
          cmake,
          autoPatchelfHook,
          fetchFromGitHub,
        }:
        stdenv.mkDerivation {
          name = "breakid";
          src = fs.toSource {
            root = ./.;
            fileset = fs.unions [
              ./CMakeLists.txt
              ./cmake
              ./src
              ./breakidConfig.cmake.in
            ];
          };
          nativeBuildInputs = [
            cmake
            autoPatchelfHook
          ];
          postInstall = ''mv $out/include/breakid/* $out/include/'';
        };

    in
    {
      packages = forAllSystems (
        system:
        let
          breakid = nixpkgsFor.${system}.callPackage breakid-package { };
        in
        {
          inherit breakid;
          default = breakid;
        }
      );
    };
}
