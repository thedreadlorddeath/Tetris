Add-Type -AssemblyName System.Drawing

function Generate-CyberpunkIcon {
    param(
        [int]$Size,
        [bool]$IsRound = $false
    )

    $bmp = New-Object System.Drawing.Bitmap($Size, $Size, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality

    $scale = [float]$Size / 512.0

    # Clip for rounded rectangle or circle
    $path = New-Object System.Drawing.Drawing2D.GraphicsPath
    if ($IsRound) {
        $path.AddEllipse([float](2 * $scale), [float](2 * $scale), [float]($Size - 4 * $scale), [float]($Size - 4 * $scale))
    } else {
        $radius = 96.0 * $scale
        $rectX = 2.0 * $scale
        $rectY = 2.0 * $scale
        $rectW = $Size - 4.0 * $scale
        $rectH = $Size - 4.0 * $scale
        $diameter = $radius * 2.0

        $path.AddArc($rectX, $rectY, $diameter, $diameter, 180, 90)
        $path.AddArc($rectX + $rectW - $diameter, $rectY, $diameter, $diameter, 270, 90)
        $path.AddArc($rectX + $rectW - $diameter, $rectY + $rectH - $diameter, $diameter, $diameter, 0, 90)
        $path.AddArc($rectX, $rectY + $rectH - $diameter, $diameter, $diameter, 90, 90)
        $path.CloseFigure()
    }
    $g.SetClip($path)

    # 1. Dark Cyberpunk Gradient Background
    $pt1 = New-Object System.Drawing.PointF(0, 0)
    $pt2 = New-Object System.Drawing.PointF($Size, $Size)
    $cTop = [System.Drawing.Color]::FromArgb(255, 11, 14, 23)   # #0B0E17 Deep Midnight
    $cBot = [System.Drawing.Color]::FromArgb(255, 26, 18, 48)   # #1A1230 Dark Violet
    $bgBrush = New-Object System.Drawing.Drawing2D.LinearGradientBrush($pt1, $pt2, $cTop, $cBot)
    $g.FillPath($bgBrush, $path)
    $bgBrush.Dispose()

    # 2. Futuristic Grid Pattern
    $gridPen = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(35, 0, 240, 255), [Math]::Max(1.0, 1.2 * $scale))
    $gridStep = 40.0 * $scale
    for ($x = 0; $x -lt $Size; $x += $gridStep) {
        $g.DrawLine($gridPen, [float]$x, 0.0, [float]$x, [float]$Size)
    }
    for ($y = 0; $y -lt $Size; $y += $gridStep) {
        $g.DrawLine($gridPen, 0.0, [float]$y, [float]$Size, [float]$y)
    }
    $gridPen.Dispose()

    # Helper function for blocks
    function Draw-3DBlock([float]$bx, [float]$by, [float]$bSize, [System.Drawing.Color]$mainCol) {
        # Base solid fill
        $brush = New-Object System.Drawing.SolidBrush($mainCol)
        $g.FillRectangle($brush, $bx, $by, $bSize, $bSize)
        $brush.Dispose()

        # 3D Top-Left Bevel Highlight
        $hiCol = [System.Drawing.Color]::FromArgb(180, 255, 255, 255)
        $hiPen = New-Object System.Drawing.Pen($hiCol, [Math]::Max(1.0, 4.0 * $scale))
        $g.DrawLine($hiPen, [float]($bx + 2 * $scale), [float]($by + 2 * $scale), [float]($bx + $bSize - 2 * $scale), [float]($by + 2 * $scale))
        $g.DrawLine($hiPen, [float]($bx + 2 * $scale), [float]($by + 2 * $scale), [float]($bx + 2 * $scale), [float]($by + $bSize - 2 * $scale))
        $hiPen.Dispose()

        # 3D Bottom-Right Bevel Shadow
        $shCol = [System.Drawing.Color]::FromArgb(140, 0, 0, 0)
        $shPen = New-Object System.Drawing.Pen($shCol, [Math]::Max(1.0, 4.0 * $scale))
        $g.DrawLine($shPen, [float]($bx + 2 * $scale), [float]($by + $bSize - 2 * $scale), [float]($bx + $bSize - 2 * $scale), [float]($by + $bSize - 2 * $scale))
        $g.DrawLine($shPen, [float]($bx + $bSize - 2 * $scale), [float]($by + 2 * $scale), [float]($bx + $bSize - 2 * $scale), [float]($by + $bSize - 2 * $scale))
        $shPen.Dispose()

        # Outline
        $outPen = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(255, 10, 15, 25), [Math]::Max(1.0, 2.0 * $scale))
        $g.DrawRectangle($outPen, $bx, $by, $bSize, $bSize)
        $outPen.Dispose()
    }

    # 3. Interlocking Neon Tetromino Composition
    $bs = 72.0 * $scale
    $ox = 144.0 * $scale
    $oy = 155.0 * $scale

    $colCyan   = [System.Drawing.Color]::FromArgb(255, 0, 240, 255)    # I-Piece
    $colViolet = [System.Drawing.Color]::FromArgb(255, 175, 60, 255)   # T-Piece
    $colGreen  = [System.Drawing.Color]::FromArgb(255, 50, 240, 80)    # S-Piece
    $colOrange = [System.Drawing.Color]::FromArgb(255, 255, 130, 20)   # L-Piece
    $colGold   = [System.Drawing.Color]::FromArgb(255, 255, 215, 0)    # O-Piece

    # Bottom Foundation: Cyan I & Green S & Orange L
    Draw-3DBlock ($ox - $bs * 0.75) ($oy + $bs * 2) $bs $colOrange
    Draw-3DBlock ($ox + $bs * 0.25) ($oy + $bs * 2) $bs $colCyan
    Draw-3DBlock ($ox + $bs * 1.25) ($oy + $bs * 2) $bs $colCyan
    Draw-3DBlock ($ox + $bs * 2.25) ($oy + $bs * 2) $bs $colGreen
    Draw-3DBlock ($ox + $bs * 2.25) ($oy + $bs * 1) $bs $colGreen

    # Centerpiece: Hero T-Piece in Neon Violet
    Draw-3DBlock ($ox + 0.0)      ($oy + 0.0) $bs $colViolet
    Draw-3DBlock ($ox + $bs)      ($oy + 0.0) $bs $colViolet
    Draw-3DBlock ($ox + $bs * 2)  ($oy + 0.0) $bs $colViolet
    Draw-3DBlock ($ox + $bs)      ($oy + $bs) $bs $colViolet

    # 4. Outer Cyber Neon Frame
    $borderPen = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(240, 0, 240, 255), [Math]::Max(2.0, 8.0 * $scale))
    $g.DrawPath($borderPen, $path)
    $borderPen.Dispose()

    $innerBorderPen = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(120, 175, 60, 255), [Math]::Max(1.0, 3.0 * $scale))
    $g.DrawPath($innerBorderPen, $path)
    $innerBorderPen.Dispose()

    $path.Dispose()
    $g.Dispose()

    return $bmp
}

$resDir = "android\app\src\main\res"

$densities = @(
    @{ Folder = "mipmap-mdpi-v4";    Size = 48 },
    @{ Folder = "mipmap-hdpi-v4";    Size = 72 },
    @{ Folder = "mipmap-xhdpi-v4";   Size = 96 },
    @{ Folder = "mipmap-xxhdpi-v4";  Size = 144 },
    @{ Folder = "mipmap-xxxhdpi-v4"; Size = 192 }
)

Write-Host "Generating Cyberpunk Tetris Android Icons..."
foreach ($d in $densities) {
    $dir = Join-Path $resDir $d.Folder
    New-Item -ItemType Directory -Force -Path $dir | Out-Null

    $icon = Generate-CyberpunkIcon -Size $d.Size -IsRound $false
    $iconPath = Join-Path $dir "ic_launcher.png"
    $icon.Save($iconPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $icon.Dispose()

    $roundIcon = Generate-CyberpunkIcon -Size $d.Size -IsRound $true
    $roundPath = Join-Path $dir "ic_launcher_round.png"
    $roundIcon.Save($roundPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $roundIcon.Dispose()

    Write-Host "  -> $($d.Folder): $($d.Size)x$($d.Size)"
}

$drawableDir = Join-Path $resDir "drawable"
New-Item -ItemType Directory -Force -Path $drawableDir | Out-Null
$appIcon = Generate-CyberpunkIcon -Size 512 -IsRound $false
$appIconPath = Join-Path $drawableDir "app_icon.png"
$appIcon.Save($appIconPath, [System.Drawing.Imaging.ImageFormat]::Png)
$appIcon.Dispose()
Write-Host "  -> drawable/app_icon.png (512x512)"

Copy-Item $appIconPath "assets\app_icon.png" -Force
Write-Host "  -> assets/app_icon.png"
Write-Host "All icons generated successfully!"
