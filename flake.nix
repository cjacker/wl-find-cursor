{
  inputs.nixpkgs.url = "nixpkgs/nixos-25.05";

  outputs = { self, nixpkgs, ... }@inputs:
  let
    pkgs = import nixpkgs { system = "x86_64-linux"; };
  in {
    packages.x86_64-linux = rec {
      default = (pkgs.callPackage ./wl-find-cursor.nix {});
    };
  };
}
