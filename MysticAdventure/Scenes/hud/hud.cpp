extends CanvasLayer

@onready var player_portrait = get_node_or_null("TopBar/PlayerPanel/PlayerPortrait")
@onready var player_health_label = get_node_or_null("TopBar/PlayerPanel/PlayerHealthLabel")

@onready var enemy_portrait = get_node_or_null("TopBar/EnemyPanel/EnemyPortrait")
@onready var enemy_health_label = get_node_or_null("TopBar/EnemyPanel/EnemyHealthLabel")

@onready var skill_disparo = get_node_or_null("SkillsPanel/SkillDisparo")
@onready var skill_rajada = get_node_or_null("SkillsPanel/SkillRajada")
@onready var skill_fogo = get_node_or_null("SkillsPanel/SkillFogo")
@onready var skill_fenomenal = get_node_or_null("SkillsPanel/SkillFenomenal")

var player_ref = null
var enemy_ref = null

func setup(player, enemy, player_texture: Texture2D, enemy_texture: Texture2D):
	player_ref = player
	enemy_ref = enemy
	
	if player_portrait != null and player_texture != null:
		player_portrait.texture = player_texture
	
	if enemy_portrait != null and enemy_texture != null:
		enemy_portrait.texture = enemy_texture

func _process(delta):
	if player_ref != null and is_instance_valid(player_ref):
		update_player_ui()
		update_skills_ui()
	
	if enemy_ref != null and is_instance_valid(enemy_ref):
		update_enemy_ui()
	elif enemy_health_label != null:
		enemy_health_label.text = "HP: 0 / 0"

func update_player_ui():
	if player_health_label != null:
		player_health_label.text = "HP: %d / %d" % [player_ref.life, player_ref.max_health]

func update_enemy_ui():
	if enemy_health_label != null:
		enemy_health_label.text = "HP: %d / %d" % [enemy_ref.life, enemy_ref.max_health]

func update_skills_ui():
	update_skill(skill_disparo, "disparos_magicos")
	update_skill(skill_rajada, "rajada_gelada")
	update_skill(skill_fogo, "bola_de_fogo")
	update_skill(skill_fenomenal, "poder_fenomenal")

func update_skill(skill_node: Control, power_name: String):
	if skill_node == null:
		return
	
	if not player_ref.unlocked_powers.has(power_name):
		skill_node.visible = false
		return
	
	if not player_ref.unlocked_powers[power_name]:
		skill_node.visible = false
		return
	
	skill_node.visible = true
	
	var icon = skill_node.get_node_or_null("Icon")
	var cooldown_label = skill_node.get_node_or_null("CooldownLabel")
	var remaining = player_ref.power_cooldown_remaining[power_name]
	
	if player_ref.power_ready[power_name]:
		if cooldown_label != null:
			cooldown_label.visible = false
		if icon != null:
			icon.modulate.a = 1.0
	else:
		if cooldown_label != null:
			cooldown_label.visible = true
			cooldown_label.text = str(int(ceil(remaining)))
		if icon != null:
			icon.modulate.a = 0.5
