{
  description = "pinc environment setup";

  nixConfig = {
    extra-substituters = [ "https://pinnout.cachix.org" ];
    extra-trusted-public-keys = [ "pinnout.cachix.org-1:51YtDHyrWhyzFO9Q8TCzF5Fb0BNBu3s5t58ovErMsns=" ];
  };

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/55bfb4d7437bd4e09ab7cba58a43bd68eb310044";
  };

  outputs = { self, nixpkgs, ... }:

    #---------------------------------------------------------------------------
    # Define parameters
    #---------------------------------------------------------------------------
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};

      basePackages = with pkgs; [
        cmake
        ninja
        python3
        python3Packages.lit
        stablehlo
        stablehloMlir
      ];

      hook = ''
export MLIR_DIR="${stablehloMlir}/lib/cmake/mlir"
export LLVM_EXTERNAL_LIT="${pkgs.python3Packages.lit}/bin/lit"
     '';
      
      #---------------------------------------------------------------------------
      # Build Packages
      #---------------------------------------------------------------------------
      stablehloMlir = pkgs.stdenv.mkDerivation{
        pname= "stablehlo_mlir";
        version="23-aab5db6";

        src = pkgs.fetchFromGitHub{
          owner = "llvm";
          repo = "llvm-project";
          rev = "aab5db6ab0702d190dbf0f3f7407d87f570c44f6";
          hash = "sha256-Jg+f+5BWQknyYCU4RAEFbuVsiKOOy7ZCS/wnmymJC60=";
        };

        nativeBuildInputs = with pkgs; [ cmake ninja python3 ];
        cmakeDir = "../llvm";
        cmakeFlags = [ "-DLLVM_INSTALL_UTILS=ON"
                       "-DLLVM_ENABLE_LLD=OFF" 
                       "-DLLVM_ENABLE_PROJECTS=mlir"
                       "-DLLVM_TARGETS_TO_BUILD=host"
                       "-DLLVM_INCLUDE_TOOLS=ON"
                       "-DLLVM_ENABLE_BINDINGS=OFF"
                       "-DLLVM_VERSION_SUFFIX="
                       "-DCMAKE_PLATFORM_NO_VERSIONED_SONAME:BOOL=ON"
                       "-DLLVM_BUILD_TOOLS=ON"
                       "-DLLVM_INCLUDE_TESTS=OFF"
                       "-DLLVM_USE_SPLIT_DWARF=ON"
                       "-DLLVM_ENABLE_ASSERTIONS=ON"
                       "-DLLVM_INCLUDE_BENCHMARKS=OFF"
                     ];
      };

      stablehlo = pkgs.stdenv.mkDerivation {
        pname = "stablehlo";
        version = "1-9fe4a1e";

        src = pkgs.fetchFromGitHub {
          owner = "openxla";
          repo = "stablehlo";
          rev = "9fe4a1e9cedbe466af2d620c2fcf87459df32eb6";
          hash = "sha256-XCgvsgxO7/Efgl58Af+eRKJi6CKeLqDIwklL/Fc6N94=";
        };
        
        nativeBuildInputs = with pkgs; [ cmake ninja python3 ];

        cmakeFlags = [
          "-DLLVM_ENABLE_ASSERTIONS=ON"
          "-DSTABLEHLO_ENABLE_BINDINGS_PYTHON=OFF"
          "-DMLIR_DIR=${stablehloMlir}/lib/cmake/mlir"
        ];

        # Only needed if StableHLO's CMake has no install() rule for its tools.
        # The cmake hook runs this from inside the build directory.
        installPhase = ''
    runHook preInstall
    install -Dm755 bin/stablehlo-opt       $out/bin/stablehlo-opt
    install -Dm755 bin/stablehlo-translate $out/bin/stablehlo-translate
    runHook postInstall
  '';
      };

      
      pinc-opt = pkgs.stdenv.mkDerivation{
        pname = "pinc-opt";
        version = "0.0.1";

        src = self;
        nativeBuildInputs = with pkgs; [ cmake ninja python3 stablehloMlir];
        cmakeFlags = [ "-DMLIR_DIR=${stablehloMlir}/lib/cmake/mlir"
                       "-DLLVM_EXTERNAL_LIT=${pkgs.python3Packages.lit}/bin/lit"
                     ];
        installPhase = ''
    runHook preInstall
    install -Dm755 bin/pinc-opt       $out/bin/pinc-opt
    runHook postInstall
  '';
      };

      #---------------------------------------------------------------------------
      # Declare devShells
      #---------------------------------------------------------------------------
    in with pkgs; {
      devShells.${system} = {
        default = mkShell {
          packages = basePackages;
          shellHook = hook;
        };
      };
      packages.${system}= {
        stablehlo-mlir = stablehloMlir;
        stablehlo = stablehlo;
        pinc-opt = pinc-opt;
      };
    };
}
