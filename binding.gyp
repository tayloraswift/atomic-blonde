{
    "targets": [{
        "target_name": "blonde", 
        "include_dirs": [
            "<!(node -e \"require('nan')\")"
        ],
        "sources": [
            "c/blonde.c"
        ], 
        "libraries": [
            "-L$HOME/tools/swift/usr/lib", 
            "-L$HOME/tools/swift/usr/lib/swift/linux", 
            "-lsourcekitdInProc"
        ]
    }]
}
