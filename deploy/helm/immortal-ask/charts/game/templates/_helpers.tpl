{{- define "game.fullname" -}}
{{- printf "%s-game" .Release.Name | trunc 63 | trimSuffix "-" -}}
{{- end -}}
