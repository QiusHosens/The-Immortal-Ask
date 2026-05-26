{{- define "auth.fullname" -}}
{{- printf "%s-auth" .Release.Name | trunc 63 | trimSuffix "-" -}}
{{- end -}}
