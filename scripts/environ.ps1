# powershell environment for ammo

function get_release_tag {
  git describe --long --match 'release-*' | %{ return $_; }
}

function get_version_full {
  get_release_tag | %{ $partone = $_.Split('-')[1]; $parttwo = $_.Split('-')[2]; return "$partone-$parttwo"; }
}

function get_version_base {
  get_release_tag | %{ $_.Split('-')[1]; }
}

function get_version_major {
  get_version_base | %{ $_.Split('.')[0]; }
}

function get_version_minor {
  get_version_base | %{ $_.Split('.')[1]; }
}

function get_version_micro {
  get_version_base | %{ $_.Split('.')[2]; }
}

function get_version_distance {
  get_release_tag | %{ $_.Split('-')[2]; }
}

function get_version_hash {
  get_release_tag | %{ $_.Split('-')[3]; }
}

function get_version_branch {
  if ( $env:IS_JENKINS -eq $null ) {
    git branch | Select-String("^\*") | %{ $_.ToString().Split(' ')[1]; }
  }
  else {
    if ( $env:GIT_BRANCH -eq $null ) {
      echo UnknownBranch
    }
    else {
      echo $env:GIT_BRANCH
    }
  }
}

