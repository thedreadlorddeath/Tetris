# Tetris Android Pre-Delivery Verification Suite (All 6 Checks)
$ErrorActionPreference = "Stop"

$readelf = "C:\Users\Thedr\AppData\Local\Temp\android-ndk-r26b\toolchains\llvm\prebuilt\windows-x86_64\bin\llvm-readelf.exe"
$aapt2 = "C:\Users\Thedr\AppData\Local\Temp\aapt2_bin\aapt2.exe"
$zipalign = "C:\Users\Thedr\AppData\Local\Temp\uber_extracted\win-zipalign_33_0_2.exe"
$uberSigner = "C:\Users\Thedr\AppData\Local\Temp\uber-apk-signer.jar"
$apk = "Tetris.apk"
$arm64So = "build-apk\stage\lib\arm64-v8a\libTetris.so"

Write-Host "================================================================================"
Write-Host "CHECK 1: DYNAMIC DEPENDENCY CHECK (DT_NEEDED on libTetris.so)"
Write-Host "Proving libsfml-activity.so is NOT in the NEEDED list"
Write-Host "================================================================================"
$needed = & $readelf -d $arm64So | Select-String -Pattern "NEEDED"
$needed | ForEach-Object { Write-Host $_ }
$hasSfmlActivity = ($needed | Select-String -Pattern "libsfml-activity.so").Count -gt 0
Write-Host "`nIs libsfml-activity.so in DT_NEEDED? $hasSfmlActivity (Expected: False)"

Write-Host "`n================================================================================"
Write-Host "CHECK 2: EXPORTED SYMBOLS CHECK (libTetris.so arm64-v8a)"
Write-Host "Confirming ANativeActivity_onCreate and main are exported"
Write-Host "================================================================================"
$symbols = & $readelf -s $arm64So | Select-String -Pattern "ANativeActivity_onCreate|main"
$symbols | ForEach-Object { Write-Host $_ }

$hasOnCreate = ($symbols | Select-String -Pattern "ANativeActivity_onCreate").Count -gt 0
$hasMain = ($symbols | Select-String -Pattern "\s+main$").Count -gt 0
Write-Host "`nANativeActivity_onCreate exported: $hasOnCreate"
Write-Host "main exported: $hasMain"

Write-Host "`n================================================================================"
Write-Host "CHECK 3: DEX CLASSLOADER CHECK (classes.dex string inspection)"
Write-Host "Verifying exact library loading order without sfml-activity"
Write-Host "================================================================================"
python -c @"
with open('build-apk/classes.dex', 'rb') as f:
    data = f.read()

libs = [b'c++_shared', b'openal', b'sfml-system', b'sfml-window', b'sfml-graphics', b'sfml-audio', b'Tetris', b'sfml-activity']
print('DEX library strings presence:')
for lib in libs:
    present = lib in data
    print(f'  {lib.decode():<18} : present={present}')
"@

Write-Host "`n================================================================================"
Write-Host "CHECK 4: COMPRESSION CHECK (APK Zip Table Inspection)"
Write-Host "Confirming resources.arsc and assets/font.ttf are stored uncompressed (stor)"
Write-Host "================================================================================"
python -c @"
import zipfile

with zipfile.ZipFile('Tetris.apk', 'r') as z:
    for info in z.infolist():
        comp_type = 'ZIP_STORED (stor)' if info.compress_type == 0 else f'ZIP_DEFLATED (defl) type {info.compress_type}'
        if info.filename in ['resources.arsc', 'classes.dex', 'AndroidManifest.xml', 'assets/font.ttf']:
            print(f'  {info.filename:<25} -> {comp_type:<20} size={info.file_size}')
        elif info.filename.startswith('lib/arm64-v8a/'):
            print(f'  {info.filename:<25} -> {comp_type:<20} size={info.file_size}')
"@

Write-Host "`n================================================================================"
Write-Host "CHECK 5: ALIGNMENT CHECK (zipalign -c -v 4)"
Write-Host "================================================================================"
& $zipalign -c -v 4 $apk

Write-Host "`n================================================================================"
Write-Host "CHECK 6: SIGNATURE CHECK (apksigner verify --verbose)"
Write-Host "================================================================================"
java -cp $uberSigner com.android.apksigner.ApkSignerTool verify --verbose $apk
