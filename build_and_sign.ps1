# Full Android Clean Build, Package & Sign Pipeline
# Follows Android 14+ / API 34 strict rules, direct sfml-main bootstrap, and 64-bit ABI support.

$ErrorActionPreference = "Stop"

$aapt2 = "C:\Users\Thedr\AppData\Local\Temp\aapt2_bin\aapt2.exe"
$androidJar = "C:\Users\Thedr\AppData\Local\Temp\android.jar"
$ndk = "C:\Users\Thedr\AppData\Local\Temp\android-ndk-r26b"
$jar = "C:\Program Files\Eclipse Adoptium\jdk-21.0.13.9-hotspot\bin\jar.exe"
$javac = "C:\Program Files\Eclipse Adoptium\jdk-21.0.13.9-hotspot\bin\javac.exe"
$r8 = "C:\Users\Thedr\AppData\Local\Temp\r8.jar"
$zipalign = "C:\Users\Thedr\AppData\Local\Temp\uber_extracted\win-zipalign_33_0_2.exe"
$uberSigner = "C:\Users\Thedr\AppData\Local\Temp\uber-apk-signer.jar"

Write-Host "=================================================================="
Write-Host "1. Recompiling Java code and regenerating classes.dex..."
Write-Host "=================================================================="
Remove-Item -Recurse -Force "build-apk\classes" -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path "build-apk\classes" | Out-Null
& $javac -cp $androidJar -d build-apk\classes android\app\src\main\java\com\thedreadlorddeath\tetris\MainActivity.java
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$classFiles = (Get-ChildItem -Recurse build-apk\classes -Filter *.class).FullName
java -cp $r8 com.android.tools.r8.D8 --min-api 21 --lib $androidJar --output build-apk $classFiles
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "=================================================================="
Write-Host "2. Recompiling Native C++ Libraries for all 3 ABIs..."
Write-Host "=================================================================="
$buildDirs = @("build-android-arm64", "build-android-armv7", "build-android-x86_64")
foreach ($b in $buildDirs) {
    Write-Host "Building $b..."
    cmake --build $b -j
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

Write-Host "=================================================================="
Write-Host "3. Compiling resources with aapt2..."
Write-Host "=================================================================="
New-Item -ItemType Directory -Force -Path "build-apk\assets" | Out-Null
Copy-Item "assets\font.ttf" "build-apk\assets\font.ttf" -Force

& $aapt2 compile --dir android\app\src\main\res -o build-apk\compiled_res.zip
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "=================================================================="
Write-Host "4. Linking resources into base.apk (uncompressed resources.arsc & font)..."
Write-Host "=================================================================="
& $aapt2 link -I $androidJar --manifest android\app\src\main\AndroidManifest.xml -o build-apk\base.apk -R build-apk\compiled_res.zip -A build-apk\assets -0 arsc -0 ttf --auto-add-overlay --min-sdk-version 21 --target-sdk-version 34 --version-code 1 --version-name "1.0.0"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "=================================================================="
Write-Host "5. Adding classes.dex to base.apk..."
Write-Host "=================================================================="
& $jar -u0f build-apk\base.apk -C build-apk classes.dex
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "=================================================================="
Write-Host "6. Staging native libraries (WITHOUT libsfml-activity.so)..."
Write-Host "=================================================================="
Remove-Item -Recurse -Force "build-apk\stage" -ErrorAction SilentlyContinue

$abis = @("arm64-v8a", "armeabi-v7a", "x86_64")
$triples = @{
    "arm64-v8a" = "aarch64-linux-android";
    "armeabi-v7a" = "arm-linux-androideabi";
    "x86_64" = "x86_64-linux-android"
}
$bMap = @{
    "arm64-v8a" = "build-android-arm64";
    "armeabi-v7a" = "build-android-armv7";
    "x86_64" = "build-android-x86_64"
}

foreach ($abi in $abis) {
    $stageDir = "build-apk\stage\lib\$abi"
    New-Item -ItemType Directory -Force -Path $stageDir | Out-Null
    
    $bDir = $bMap[$abi]
    # Game library with sfml-main ANativeActivity_onCreate exported
    Copy-Item "$bDir\libTetris.so" "$stageDir\libTetris.so" -Force
    
    # SFML component libraries (DO NOT copy libsfml-activity.so!)
    Copy-Item "$bDir\_deps\sfml-build\lib\libsfml-audio.so" "$stageDir\" -Force
    Copy-Item "$bDir\_deps\sfml-build\lib\libsfml-graphics.so" "$stageDir\" -Force
    Copy-Item "$bDir\_deps\sfml-build\lib\libsfml-network.so" "$stageDir\" -Force
    Copy-Item "$bDir\_deps\sfml-build\lib\libsfml-system.so" "$stageDir\" -Force
    Copy-Item "$bDir\_deps\sfml-build\lib\libsfml-window.so" "$stageDir\" -Force
    
    # OpenAL
    Copy-Item "build\_deps\sfml-src\extlibs\libs-android\$abi\libopenal.so" "$stageDir\" -Force
    
    # NDK C++ STL runtime
    $triple = $triples[$abi]
    Copy-Item "$ndk\toolchains\llvm\prebuilt\windows-x86_64\sysroot\usr\lib\$triple\libc++_shared.so" "$stageDir\" -Force
    
    Write-Host "   Staged $abi : $((Get-ChildItem $stageDir).Count) libraries (libsfml-activity excluded)"
}

Write-Host "=================================================================="
Write-Host "7. Adding staged lib/ to base.apk..."
Write-Host "=================================================================="
& $jar -u0f build-apk\base.apk -C build-apk\stage lib
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "=================================================================="
Write-Host "8. Aligning APK with zipalign (4-byte alignment)..."
Write-Host "=================================================================="
& $zipalign -f -v 4 build-apk\base.apk build-apk\aligned.apk
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "=================================================================="
Write-Host "9. Signing APK with dedicated release keystore (v1, v2, v3)..."
Write-Host "=================================================================="
java -cp $uberSigner com.android.apksigner.ApkSignerTool sign --ks android\release.keystore --ks-key-alias tetris_release --ks-pass pass:tetris2026 --key-pass pass:tetris2026 --v1-signing-enabled true --v2-signing-enabled true --v3-signing-enabled true --out build-apk\Tetris-signed.apk build-apk\aligned.apk
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Copy-Item "build-apk\Tetris-signed.apk" "android\Tetris.apk" -Force
Copy-Item "build-apk\Tetris-signed.apk" "Tetris.apk" -Force

Write-Host "BUILD SUCCESSFUL! Final APK ready at android/Tetris.apk and Tetris.apk"
