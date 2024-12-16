extends Cabin3D

func _on_czuwak_blink(state):
    $Lights/CzuwakOmni1.enabled = state
    $Lights/CzuwakOmni2.enabled = state
    $Lights/CzuwakOmni3.enabled = state
