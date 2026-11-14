$readelf = "C:\Users\Thedr\AppData\Local\Temp\android-ndk-r26b\toolchains\llvm\prebuilt\windows-x86_64\bin\llvm-readelf.exe"
$aapt2 = "C:\Users\Thedr\AppData\Local\Temp\aapt2_bin\aapt2.exe"
$zipalign = "C:\Users\Thedr\AppData\Local\Temp\uber_extracted\win-zipalign_33_0_2.exe"
$uberSigner = "C:\Users\Thedr\AppData\Local\Temp\uber-apk-signer.jar"
$apk = "Tetris.apk"

Write-Host "================================================================================"
Write-Host "CHECK 1: SYMBOL & ENTRY POINT CHECK (llvm-readelf -s)"
Write-Host "================================================================================"
Write-Host "[1A] libsfml-activity.so (arm64-v8a):"
& $readelf -s build-apk\stage\lib\arm64-v8a\libsfml-activity.so | Select-String -Pattern "ANativeActivity_onCreate|main"

Write-Host "`n[1B] libTetris.so (arm64-v8a):"
& $readelf -s build-apk\stage\lib\arm64-v8a\libTetris.so | Select-String -Pattern "ANativeActivity_onCreate|main"

Write-Host "`n================================================================================"
Write-Host "CHECK 2: COMPRESSION & CONTENTS CHECK (ZIP Table Inspection)"
Write-Host "================================================================================"
python -c @"
import zipfile

with zipfile.ZipFile('Tetris.apk', 'r') as z:
    infolist = z.infolist()
    print(f'Total entries in APK: {len(infolist)}')
    print('Key entries compression check:')
    for info in infolist:
        comp_type = 'ZIP_STORED (stor)' if info.compress_type == 0 else f'ZIP_DEFLATED (defl) type {info.compress_type}'
        if info.filename in ['resources.arsc', 'classes.dex', 'AndroidManifest.xml', 'assets/font.ttf']:
            print(f'  {info.filename:<25} -> {comp_type:<20} size={info.file_size}')
        elif info.filename.startswith('lib/arm64-v8a/'):
            print(f'  {info.filename:<25} -> {comp_type:<20} size={info.file_size}')
"@

Write-Host "`n================================================================================"
Write-Host "CHECK 3: BADGING CHECK (aapt2 dump badging)"
Write-Host "================================================================================"
& $aapt2 dump badging $apk

Write-Host "`n================================================================================"
Write-Host "CHECK 4: ALIGNMENT CHECK (zipalign -c -v 4)"
Write-Host "================================================================================"
& $zipalign -c -v 4 $apk

Write-Host "`n================================================================================"
Write-Host "CHECK 5: SIGNATURE CHECK (apksigner verify --verbose)"
Write-Host "================================================================================"
java -cp $uberSigner com.android.apksigner.ApkSignerTool verify --verbose $apk
