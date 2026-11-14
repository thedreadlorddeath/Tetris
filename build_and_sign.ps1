# Tetris Android Packaging and Signing Script
# Uses standard aapt2, jar, zipalign, and ApkSignerTool pipeline

$aapt2 = "C:\Users\Thedr\AppData\Local\Temp\aapt2_bin\aapt2.exe"
$androidJar = "C:\Users\Thedr\AppData\Local\Temp\android.jar"
$ndk = "C:\Users\Thedr\AppData\Local\Temp\android-ndk-r26b"
$jar = "C:\Program Files\Eclipse Adoptium\jdk-21.0.13.9-hotspot\bin\jar.exe"
$zipalign = "C:\Users\Thedr\AppData\Local\Temp\uber_extracted\win-zipalign_33_0_2.exe"
$uberSigner = "C:\Users\Thedr\AppData\Local\Temp\uber-apk-signer.jar"

New-Item -ItemType Directory -Force -Path "build-apk\assets" | Out-Null
Copy-Item "assets\font.ttf" "build-apk\assets\font.ttf" -Force

Write-Host "1. Compiling resources with aapt2..."
& $aapt2 compile --dir android/app/src/main/res -o build-apk/compiled_res.zip
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "2. Linking resources into base.apk (uncompressed resources.arsc)..."
& $aapt2 link -I $androidJar --manifest android/app/src/main/AndroidManifest.xml -o build-apk/base.apk -R build-apk/compiled_res.zip -A build-apk/assets -0 arsc -0 ttf --auto-add-overlay --min-sdk-version 21 --target-sdk-version 34 --version-code 1 --version-name "1.0.0"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "3. Adding classes.dex to base.apk..."
& $jar -u0f build-apk/base.apk -C build-apk classes.dex
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "4. Staging native libraries (arm64-v8a, armeabi-v7a, x86_64)..."
$abis = @("arm64-v8a", "armeabi-v7a", "x86_64")
$triples = @{
    "arm64-v8a" = "aarch64-linux-android";
    "armeabi-v7a" = "arm-linux-androideabi";
    "x86_64" = "x86_64-linux-android"
}
$buildDirs = @{
    "arm64-v8a" = "build-android-arm64";
    "armeabi-v7a" = "build-android-armv7";
    "x86_64" = "build-android-x86_64"
}

foreach ($abi in $abis) {
    $stageDir = "build-apk\stage\lib\$abi"
    New-Item -ItemType Directory -Force -Path $stageDir | Out-Null
    
    $bDir = $buildDirs[$abi]
    Copy-Item "$bDir\libTetris.so" "$stageDir\libTetris.so" -Force
    Copy-Item "$bDir\_deps\sfml-build\lib\libsfml-activity.so" "$stageDir\" -Force
    Copy-Item "$bDir\_deps\sfml-build\lib\libsfml-audio.so" "$stageDir\" -Force
    Copy-Item "$bDir\_deps\sfml-build\lib\libsfml-graphics.so" "$stageDir\" -Force
    Copy-Item "$bDir\_deps\sfml-build\lib\libsfml-network.so" "$stageDir\" -Force
    Copy-Item "$bDir\_deps\sfml-build\lib\libsfml-system.so" "$stageDir\" -Force
    Copy-Item "$bDir\_deps\sfml-build\lib\libsfml-window.so" "$stageDir\" -Force
    Copy-Item "build\_deps\sfml-src\extlibs\libs-android\$abi\libopenal.so" "$stageDir\" -Force
    
    $triple = $triples[$abi]
    Copy-Item "$ndk\toolchains\llvm\prebuilt\windows-x86_64\sysroot\usr\lib\$triple\libc++_shared.so" "$stageDir\" -Force
    
    Write-Host "   Staged $abi : $((Get-ChildItem $stageDir).Count) libraries"
}

Write-Host "5. Adding lib/ to base.apk..."
& $jar -u0f build-apk/base.apk -C build-apk/stage lib
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "6. Aligning APK with zipalign (4-byte alignment)..."
& $zipalign -f -v 4 build-apk/base.apk build-apk/aligned.apk
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "7. Signing APK with dedicated release keystore (v1, v2, v3 schemes)..."
java -cp $uberSigner com.android.apksigner.ApkSignerTool sign --ks android\release.keystore --ks-key-alias tetris_release --ks-pass pass:tetris2026 --key-pass pass:tetris2026 --v1-signing-enabled true --v2-signing-enabled true --v3-signing-enabled true --out build-apk\Tetris-signed.apk build-apk\aligned.apk
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Copy-Item "build-apk\Tetris-signed.apk" "android\Tetris.apk" -Force
Copy-Item "build-apk\Tetris-signed.apk" "Tetris.apk" -Force

Write-Host "APK build complete: android/Tetris.apk and Tetris.apk"
